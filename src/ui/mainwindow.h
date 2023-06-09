#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QGraphicsScene>
#include <QMainWindow>
#include <QGraphicsPixmapItem>
#include <QStringListModel>
#include <QGraphicsSceneMouseEvent>

#include "../psd/psd_manager.h"
#include "../processing/processor_api.h"
#include "../processing/common_processors.h"

#include "proc_ctx.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class LetterRect : public QGraphicsItem
{
public:
    LetterRect(const LetterData&);

    QRectF boundingRect() const;

    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);

protected:
    static const QPen red;
    static const QPen green;

    const LetterData letter;
    QPen pen;

    void mousePressEvent(QGraphicsSceneMouseEvent*);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void open_file();
    void save_file();
    void save_file_as();

    void grayscale();
    void duotone_start();
    void duotone_try(int);
    void duotone_done();
    void duotone_cancel();
    void fill_holes();
    void thin_top();
    void thin_right();
    void thin_bottom();
    void thin_left();
    void trace_letters();

    void import_history();
    void export_history();

private:
    Ui::MainWindow *ui;

    QImage image;
    QGraphicsScene img_scene;
    QGraphicsPixmapItem* img_pixmap_item;
    QStringListModel* history_str_model;
    QStringList history_strs;

    PsdManager psd_manager;

    std::map<ProcessorType, std::unique_ptr<ImageProcessor>> processors;
    std::list<std::unique_ptr<ProcCtx>> proc_history;
    std::vector<LetterRect*> letters_meta;

    void draw_image();
    void draw_image(const ImageData&);
    void add_letter_meta(const class LetterData& letter);
    void clear_letter_meta();

    void thin_letter(BorderSide);

    void reapply_history();
};
#endif // MAIN_WINDOW_H
