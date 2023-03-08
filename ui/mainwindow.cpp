#include <QFileDialog>
#include <QMessageBox>

#include "./mainwindow.h"
#include "./ui_mainwindow.h"

// ===== local UI utility structs =====
static struct
{
    const int min = 1;
    const int max = 100;
    const int default_value = min;
    QSpinBox* spinbox = nullptr;
    QSlider* slider = nullptr;

    inline void sync_fields(int& value)
    {
        if (value > max)
            value = max;
        else if (value < min)
            value = min;

        slider->setValue(value);
        spinbox->setValue(value);
    }
} scale_ctx;

static struct
{
    QWidget* widget_img_info;
    QAction* action_save_as;
    QAction* action_save;

    inline void no_img()
    {
        widget_img_info->setVisible(false);
        action_save_as->setEnabled(false);
        action_save->setEnabled(false);
    }

    inline void img_opened()
    {
        widget_img_info->setVisible(true);
        action_save_as->setEnabled(true);
        action_save->setEnabled(false);
    }

    inline void img_saved()
    {
        widget_img_info->setVisible(true);
        action_save_as->setEnabled(true);
        action_save->setEnabled(true);
    }
} visibility_ctx;

static struct
{
    QLabel* label_width;
    QLabel* label_height;
    QLabel* label_depth;
    QLabel* label_channels;

    void set(const PsdData& img)
    {
        label_width->setText(QString::number(img.width));
        label_height->setText(QString::number(img.height));
        label_depth->setText(QString::number(img.depth));
        label_channels->setText(QString::number(img.n_channels));
    }
} img_info_ctx;

// ===== local PsdData to Qt's QRgb struct mappers =====
typedef QRgb (*rgb_mapper)(const PsdData& img, uint64_t idx);

static QRgb to_rgb(const PsdData& img, uint64_t idx)
{
    return qRgb(img.channels_data[0][idx], img.channels_data[1][idx],
        img.channels_data[2][idx]);
}

static QRgb to_rgba(const PsdData& img, uint64_t idx)
{
    return qRgba(img.channels_data[0][idx], img.channels_data[1][idx],
        img.channels_data[2][idx], img.channels_data[3][idx]);
}

static void map_image(const PsdData& psd_img, QImage& q_img)
{
    rgb_mapper map_pixel;
    switch (psd_img.n_channels)
    {
    case 3:
        map_pixel = to_rgb;
        break;
    case 4:
        map_pixel = to_rgba;
        break;
    default:
        // other formats aren't supported
        return;
    }

    auto width = q_img.width();

    for (int r = 0; r < q_img.height(); ++r)
    {
        QRgb *line = reinterpret_cast<QRgb*>(q_img.scanLine(r));
        for (int c = 0; c < width; ++c)
        {
            line[c] = map_pixel(psd_img, r * width + c);
        }
    }
}

// ===== class methods =====
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    image(),
    img_pixmap_item(img_scene.addPixmap(QPixmap::fromImage(image)))
{
    ui->setupUi(this);

    scale_ctx.slider = ui->slider_scale;
    scale_ctx.spinbox = ui->spinbox_scale;

    scale_ctx.spinbox->setMinimum(scale_ctx.min);
    scale_ctx.spinbox->setMaximum(scale_ctx.max);
    scale_ctx.slider->setMinimum(scale_ctx.min);
    scale_ctx.slider->setMaximum(scale_ctx.max);
    sync_scale(scale_ctx.default_value);

    visibility_ctx.widget_img_info = ui->widget_img_info;
    visibility_ctx.action_save = ui->actionSave;
    visibility_ctx.action_save_as = ui->actionSave_as;
    visibility_ctx.no_img();

    img_info_ctx.label_width = ui->label_width_data;
    img_info_ctx.label_height = ui->label_height_data;
    img_info_ctx.label_depth = ui->label_depth_data;
    img_info_ctx.label_channels = ui->label_channels_data;

    QObject::connect(scale_ctx.slider, &QSlider::valueChanged,
        this, &MainWindow::sync_scale);
    QObject::connect(scale_ctx.spinbox, &QSpinBox::valueChanged,
        this, &MainWindow::sync_scale);

    QObject::connect(ui->actionOpen, &QAction::triggered,
        this, &MainWindow::open_file);
    QObject::connect(ui->actionSave, &QAction::triggered,
        this, &MainWindow::save_file);
    QObject::connect(ui->actionSave_as, &QAction::triggered,
        this, &MainWindow::save_file_as);

    QObject::connect(ui->actionTest_Set_Red, &QAction::triggered,
        this, &MainWindow::test_set_red_80);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::draw_image()
{
    PsdData& psd_img = psd_manager.get_image();
    if (image.height() != psd_img.height || image.width() != psd_img.width)
        image = QImage(psd_img.width, psd_img.height, QImage::Format_ARGB32);

    map_image(psd_img, image);

    img_scene.clear();
    img_pixmap_item = img_scene.addPixmap(QPixmap::fromImage(image));

    img_info_ctx.set(psd_img);

    ui->image_box->setScene(&img_scene);
}

void MainWindow::sync_scale(int value)
{
    scale_ctx.sync_fields(value);

    this->img_pixmap_item->setScale(value);// TODO: works a bit weird
}

void MainWindow::open_file()
{
    QString file_name = QFileDialog::getOpenFileName(this,
        tr("Open PSD Image"), "", tr("PSD File (*.psd)"));
    if (file_name.isEmpty())
        return;
    if (!psd_manager.open(file_name.toLocal8Bit().data()))
    {
        // NOTE: could display error specific info if PsdManager was to provide it
        QMessageBox::warning(this, tr("Error opening file"),
            tr("An error occured while opening selected file."),
            QMessageBox::Ok);
        return;
    }
    draw_image();

    visibility_ctx.img_opened();

    ui->actionTest_Set_Red->setEnabled(true);

    return;
}

void MainWindow::save_file()
{
    psd_manager.save();
}

void MainWindow::save_file_as()
{
    QString file_name = QFileDialog::getSaveFileName(this,
        tr("Open PSD Image"), "", tr("PSD File (*.psd)"));
    if (file_name.isEmpty())
        return;

    psd_manager.set_save_path(file_name.toLocal8Bit().data());

    visibility_ctx.img_saved();

    save_file();
}

void MainWindow::test_set_red_80()
{
    PsdData& img = psd_manager.get_image();

    std::vector<uint8_t>& red = img.channels_data[0];
    std::fill(red.begin(), red.end(), 205);

    draw_image();
}
