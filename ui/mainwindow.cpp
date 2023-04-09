#include <QFileDialog>
#include <QMessageBox>

#include "mainwindow.h"

#include "./ui_mainwindow.h"
#include "../processing/common_processors.h"
#include "../processing/test_make_red_proc.h"

#include "utility_ctx.h"

using namespace ui_context;

static SliderSpinboxSyncCtx duotone_split_ctx(0, 255, 127);
static PixmapScaleCtx scale_ctx(0, 100, 0);

// ===== local ImageData to Qt's QRgb struct mappers =====
typedef QRgb (*rgb_mapper)(const ImageData& img, uint64_t idx);

static QRgb to_rgb(const ImageData& img, uint64_t idx)
{
    return qRgb(img.channels_data[0][idx], img.channels_data[1][idx],
        img.channels_data[2][idx]);
}

static QRgb to_rgba(const ImageData& img, uint64_t idx)
{
    return qRgba(img.channels_data[0][idx], img.channels_data[1][idx],
        img.channels_data[2][idx], img.channels_data[3][idx]);
}

static QRgb to_gray(const ImageData& img, uint64_t idx)
{
    return qRgb(img.channels_data[0][idx], img.channels_data[0][idx],
        img.channels_data[0][idx]);
}

static void map_image(const ImageData& raw_img, QImage& q_img)
{
    rgb_mapper map_pixel;
    switch (raw_img.n_channels)
    {
    case 1:
        map_pixel = to_gray;
        break;
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
            line[c] = map_pixel(raw_img, r * width + c);
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

    processors.emplace(TEST_RED_80_PERCENT, new MakeRed80Percent());
    processors.emplace(GRAYSCALE, new Grayscale());

    scale_ctx.slider = ui->slider_scale;
    scale_ctx.spinbox = ui->spinbox_scale;
    scale_ctx.pixmap = &img_pixmap_item;

    scale_ctx.spinbox->setMinimum(scale_ctx.get_min());
    scale_ctx.spinbox->setMaximum(scale_ctx.get_max());
    scale_ctx.slider->setMinimum(scale_ctx.get_min());
    scale_ctx.slider->setMaximum(scale_ctx.get_max());
    scale_ctx.sync(scale_ctx.get_default_value());

    duotone_split_ctx.slider = ui->slider_duotone;
    duotone_split_ctx.spinbox = ui->spinBox_duotone;

    duotone_split_ctx.spinbox->setMinimum(duotone_split_ctx.get_min());
    duotone_split_ctx.spinbox->setMaximum(duotone_split_ctx.get_max());
    duotone_split_ctx.slider->setMinimum(duotone_split_ctx.get_min());
    duotone_split_ctx.slider->setMaximum(duotone_split_ctx.get_max());
    duotone_split_ctx.sync(duotone_split_ctx.get_default_value());

    visibility_ctx.widget_img_info = ui->widget_img_info;
    visibility_ctx.widget_toolbox = ui->tools_tabs;
    visibility_ctx.action_save = ui->actionSave;
    visibility_ctx.action_save_as = ui->actionSave_as;
    visibility_ctx.no_img();

    duotone_visibility_ctx.widget_preview = ui->groupBox_duotone_preview;
    duotone_visibility_ctx.button_duotone = ui->button_duotone_preview;
    duotone_visibility_ctx.end_preview();

    img_info_ctx.label_width = ui->label_width_data;
    img_info_ctx.label_height = ui->label_height_data;
    img_info_ctx.label_depth = ui->label_depth_data;
    img_info_ctx.label_channels = ui->label_channels_data;

    // utility contexts
    QObject::connect(scale_ctx.slider, &QSlider::valueChanged,
        &scale_ctx, &PixmapScaleCtx::sync);
    QObject::connect(scale_ctx.spinbox, &QSpinBox::valueChanged,
        &scale_ctx, &PixmapScaleCtx::sync);

    QObject::connect(duotone_split_ctx.slider, &QSlider::valueChanged,
        &duotone_split_ctx, &SliderSpinboxSyncCtx::sync);
    QObject::connect(duotone_split_ctx.spinbox, &QSpinBox::valueChanged,
        &duotone_split_ctx, &SliderSpinboxSyncCtx::sync);

    // menu items
    QObject::connect(ui->actionOpen, &QAction::triggered,
        this, &MainWindow::open_file);
    QObject::connect(ui->actionSave, &QAction::triggered,
        this, &MainWindow::save_file);
    QObject::connect(ui->actionSave_as, &QAction::triggered,
        this, &MainWindow::save_file_as);

    // processing events
    QObject::connect(ui->actionTest_Set_Red, &QAction::triggered,
        this, &MainWindow::test_set_red_80);
    QObject::connect(ui->button_grayscale, &QAbstractButton::pressed,
        this, &MainWindow::grayscale);

    QObject::connect(ui->button_duotone_preview, &QAbstractButton::pressed,
        this, &MainWindow::duotone_start);
    QObject::connect(ui->button_duotone_done, &QAbstractButton::pressed,
        this, &MainWindow::duotone_done);
    QObject::connect(ui->button_duotone_cancel, &QAbstractButton::pressed,
        this, &MainWindow::duotone_cancel);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::draw_image()
{
    ImageData& raw_img = psd_manager.get_image().get_raw();
    if (image.height() != raw_img.height || image.width() != raw_img.width)
        image = QImage(raw_img.width, raw_img.height, QImage::Format_ARGB32);

    map_image(raw_img, image);

    img_scene.clear();
    img_pixmap_item = img_scene.addPixmap(QPixmap::fromImage(image));

    img_info_ctx.set(psd_manager.get_image());

    ui->image_box->setScene(&img_scene);
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

    if (processors[TEST_RED_80_PERCENT]->process(img.get_raw()))
        draw_image();
}

void MainWindow::grayscale()
{
    PsdData& img = psd_manager.get_image();

    if (processors[GRAYSCALE]->process(img.get_raw()))
    {
        img.set_color_mode(PsdData::ColorMode::GRAYSCALE);
        draw_image();
    }
}

void MainWindow::duotone_start()
{
    duotone_visibility_ctx.start_preview();
}

void MainWindow::duotone_done()
{
    qDebug("duotone done");
    duotone_visibility_ctx.end_preview();
}

void MainWindow::duotone_cancel()
{
    qDebug("duotone cancel");
    duotone_visibility_ctx.end_preview();
}
