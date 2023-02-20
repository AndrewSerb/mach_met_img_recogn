#include "mainwindow.h"
#include "./ui_mainwindow.h"

static struct
{
    const int min = 1;
    const int max = 10;
    const int default_value = min;
    QSpinBox* spinbox = nullptr;
    QSlider* slider = nullptr;
} scale_context;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    image(),
    img_pixmap_item(img_scene.addPixmap(QPixmap::fromImage(image)))
{
    ui->setupUi(this);

    ui->image_box->setScene(&img_scene);

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
}

MainWindow::~MainWindow()
{
    delete ui;
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
