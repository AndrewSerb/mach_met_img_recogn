#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QGraphicsScene>
#include <QMainWindow>
#include <QGraphicsPixmapItem>

#include "../psd/psd_manager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void sync_scale(int);
    void open_file();
    void save_file();
    void save_file_as();

    void test_set_red_80();

private:
    Ui::MainWindow *ui;

    QImage image;
    QGraphicsScene img_scene;
    QGraphicsPixmapItem* img_pixmap_item;

    PsdManager psd_manager;

    void draw_image();
};
#endif // MAIN_WINDOW_H
