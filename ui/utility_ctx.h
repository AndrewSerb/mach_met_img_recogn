#ifndef UTILITY_CTX_H
#define UTILITY_CTX_H

#include "qabstractbutton.h"
#include "qgraphicsitem.h"
#include "qlabel.h"
#include "qslider.h"
#include "qspinbox.h"
#include "qstringlistmodel.h"

#include "../psd/psd_manager.h"

#include "proc_ctx.h"

namespace ui_context
{
struct SliderSpinboxSyncCtx : public QObject
{
    Q_OBJECT

public:
    QSpinBox* spinbox = nullptr;
    QSlider* slider = nullptr;

    SliderSpinboxSyncCtx(int min, int max, int default_value)
    {
        if (min > max)
        {
            this->min = max;
            this->max = min;
        }
        else
        {
            this->max = max;
            this->min = min;
        }

        if (default_value > this->max)
            this->default_value = this->max;
        else if (default_value < this->min)
            this->default_value = this->min;
        else
            this->default_value = default_value;

        last_value = default_value;
    }

    inline int get_min() const
    {
        return min;
    }

    inline int get_max() const
    {
        return max;
    }

    inline int get_default_value() const
    {
        return default_value;
    }

    inline int get_last() const
    {
        return last_value;
    }

    Q_SLOT virtual void sync(int value)
    {
        if (value > max)
            value = max;
        else if (value < min)
            value = min;

        slider->setValue(value);
        spinbox->setValue(value);

        last_value = value;

        emit value_changed(value);
    }

    Q_SIGNAL void value_changed(int);

private:
    int min;
    int max;
    int default_value;
    int last_value;
};

struct PixmapScaleCtx : public SliderSpinboxSyncCtx
{
    Q_OBJECT
public:
    PixmapScaleCtx(int min, int max, int default_value)
        : SliderSpinboxSyncCtx(min, max, default_value)
    {}

    inline void sync(int value) Q_DECL_OVERRIDE
    {
        SliderSpinboxSyncCtx::sync(value);

        (*pixmap)->setScale(value);// TODO: works a bit weird
    }

    QGraphicsPixmapItem** pixmap;
};

struct
{
    QWidget* widget_img_info;
    QWidget* widget_toolbox;
    QAction* action_save_as;
    QAction* action_save;

    inline void no_img()
    {
        widget_img_info->setVisible(false);
        widget_toolbox->setVisible(false);
        action_save_as->setEnabled(false);
        action_save->setEnabled(false);
    }

    inline void img_opened()
    {
        widget_img_info->setVisible(true);
        widget_toolbox->setVisible(true);
        action_save_as->setEnabled(true);
        action_save->setEnabled(false);
    }

    inline void img_saved()
    {
        widget_img_info->setVisible(true);
        widget_toolbox->setVisible(true);
        action_save_as->setEnabled(true);
        action_save->setEnabled(true);
    }
} visibility_ctx;

struct
{
    QWidget* widget_preview;
    QAbstractButton* button_duotone;

    inline void start_preview()
    {
        widget_preview->setVisible(true);
        button_duotone->setVisible(false);
    }

    inline void end_preview()
    {
        widget_preview->setVisible(false);
        button_duotone->setVisible(true);
    }
} duotone_visibility_ctx;

struct
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

struct ProcHistoryManager
{
    QStringListModel* model;
    QStringList* list;

    void add(const ProcCtx&);
    void clear();
};

}

#endif
