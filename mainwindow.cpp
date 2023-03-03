#include <QFileDialog>
#include <QMessageBox>

#include "mainwindow.h"
#include "./ui_mainwindow.h"

static struct
{
    const int min = 1;
    const int max = 100;
    const int default_value = min;
    QSpinBox* spinbox = nullptr;
    QSlider* slider = nullptr;
} scale_context;

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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    image(),
    img_pixmap_item(img_scene.addPixmap(QPixmap::fromImage(image)))
{
    ui->setupUi(this);

    scale_context.slider = ui->slider_scale;
    scale_context.spinbox = ui->spinbox_scale;

    scale_context.spinbox->setMinimum(scale_context.min);
    scale_context.spinbox->setMaximum(scale_context.max);
    scale_context.slider->setMinimum(scale_context.min);
    scale_context.slider->setMaximum(scale_context.max);
    sync_scale(scale_context.default_value);

    QObject::connect(scale_context.slider, &QSlider::valueChanged,
        this, &MainWindow::sync_scale);
    QObject::connect(scale_context.spinbox, &QSpinBox::valueChanged,
        this, &MainWindow::sync_scale);

    QObject::connect(ui->actionOpen, &QAction::triggered,
        this, &MainWindow::open_file);
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

    ui->image_box->setScene(&img_scene);
}

void MainWindow::sync_scale(int value)
{
    if (value > scale_context.max)
        value = scale_context.max;
    else if (value < scale_context.min)
        value = scale_context.min;

    scale_context.slider->setValue(value);
    scale_context.spinbox->setValue(value);
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
        // TODO: could display error specific info if PsdManager was to provide it
        QMessageBox::warning(this, tr("Error opening file"),
            tr("An error occured while opening selected file."),
            QMessageBox::Ok);
        return;
    }
    draw_image();

    return;
}
