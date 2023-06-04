#include <QFileDialog>
#include <QMessageBox>
#include <QToolTip>
#include <QPainter>

#include "mainwindow.h"

#include "./ui_mainwindow.h"
#include "../processing/common_processors.h"

#include "utility_ctx.h"

using namespace ui_context;

static SliderSpinboxSyncCtx duotone_split_ctx(0, 255, 127);
static PixmapScaleCtx scale_ctx(1, 100, 1);
static ProcHistoryManager history_ctx;
static LetterInfoCtx letter_info_ctx;

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

    history_str_model = new QStringListModel();

    ui->listView->setModel(history_str_model);

    history_ctx.list = &history_strs;
    history_ctx.model = history_str_model;

    processors.emplace(GRAYSCALE, new Grayscale());
    processors.emplace(DUOTONE, new Duotone());
    processors.emplace(FILL, new Fill());
    processors.emplace(THIN, new ThinLetters());
    processors.emplace(IRREG_CLEANUP, new IrregCleanup());
    processors.emplace(TRACE_LETTERS, new LetterFinder());

    scale_ctx.slider = ui->slider_scale;
    scale_ctx.spinbox = ui->spinbox_scale;
    scale_ctx.view = ui->image_box;

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

    letter_info_ctx.label_hor_cnt = ui->label_hor_gr_num;
    letter_info_ctx.label_vert_cnt = ui->label_vert_groups_num;

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

    // == processing events
    // grayscale
    QObject::connect(ui->button_grayscale, &QAbstractButton::pressed,
        this, &MainWindow::grayscale);
    // duotone
    QObject::connect(ui->button_duotone_preview, &QAbstractButton::pressed,
        this, &MainWindow::duotone_start);
    QObject::connect(ui->button_duotone_done, &QAbstractButton::pressed,
        this, &MainWindow::duotone_done);
    QObject::connect(ui->button_duotone_cancel, &QAbstractButton::pressed,
        this, &MainWindow::duotone_cancel);
    QObject::connect(&duotone_split_ctx, &SliderSpinboxSyncCtx::value_changed,
        this, &MainWindow::duotone_try);
    QObject::connect(&duotone_split_ctx, &SliderSpinboxSyncCtx::value_changed,
        this, &MainWindow::duotone_try);
    // fill holes
    QObject::connect(ui->button_fill, &QAbstractButton::pressed,
        this, &MainWindow::fill_holes);
    // thin letters
    QObject::connect(ui->button_thin_top, &QAbstractButton::pressed,
        this, &MainWindow::thin_top);
    QObject::connect(ui->button_thin_right, &QAbstractButton::pressed,
        this, &MainWindow::thin_right);
    QObject::connect(ui->button_thin_bottom, &QAbstractButton::pressed,
        this, &MainWindow::thin_bottom);
    QObject::connect(ui->button_thin_left, &QAbstractButton::pressed,
        this, &MainWindow::thin_left);
    // trace letters
    QObject::connect(ui->button_trace_letters, &QAbstractButton::pressed,
        this, &MainWindow::trace_letters);

    // == history
    QObject::connect(ui->button_import_history, &QAbstractButton::pressed,
        this, &MainWindow::import_history);
    QObject::connect(ui->button_export_history, &QAbstractButton::pressed,
        this, &MainWindow::export_history);
}

MainWindow::~MainWindow()
{
    delete history_str_model;
    delete ui;
}

void MainWindow::draw_image(const ImageData& raw_img)
{
    if (image.height() != raw_img.height || image.width() != raw_img.width)
    {
        image = QImage(raw_img.width, raw_img.height, QImage::Format_ARGB32);
        clear_letter_meta();
    }

    map_image(raw_img, image);

    img_scene.clear();
    img_pixmap_item = img_scene.addPixmap(QPixmap::fromImage(image));
    img_pixmap_item->setTransformOriginPoint(img_scene.sceneRect().center());

    scale_ctx.sync(scale_ctx.get_last());
    img_info_ctx.set(psd_manager.get_image());

    ui->image_box->setScene(&img_scene);
    ui->image_box->repaint();
}

void MainWindow::add_letter_meta(const LetterData& letter)
{
    LetterRect* lr = new LetterRect(letter);
    img_scene.addItem(lr);
    letters_meta.push_back(lr);
}

void MainWindow::clear_letter_meta()
{
    for (auto letter : letters_meta)
    {
        img_scene.removeItem(letter);
    }
    letters_meta.clear();
    letter_info_ctx.clear_info();
}

void MainWindow::draw_image()
{
    draw_image(psd_manager.get_image().get_raw());
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
    proc_history.clear();
    history_ctx.clear();
    clear_letter_meta();
    draw_image();

    visibility_ctx.img_opened();

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

// preprocessing

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
    if (psd_manager.get_image().n_channels != 1)
    {
        QToolTip::showText(
            ui->button_duotone_preview->mapToGlobal(QPoint(0, 0)),
            "A grayscaled 1 channel image is expected for this action",
            ui->button_duotone_preview);
        return;
    }

    duotone_visibility_ctx.start_preview();
    duotone_try(duotone_split_ctx.get_last());
}

void MainWindow::duotone_try(int value)
{
    Duotone* duotone = (Duotone*)processors[DUOTONE].get();

    duotone->set_split_value(value);
    if (duotone->process(psd_manager.get_image().get_raw()))
        draw_image(duotone->get_preview());
}

void MainWindow::duotone_done()
{
    duotone_visibility_ctx.end_preview();

    Duotone* duotone = (Duotone*)processors[DUOTONE].get();
    psd_manager.get_image().get_raw() = duotone->get_preview();
    duotone->clear_preview();

    draw_image();

    unsigned split_value = duotone->get_split_value();

    if (!proc_history.size() || proc_history.back()->type != DUOTONE ||
        ((ThresholdActionCtx*)proc_history.back().get())->threshold != split_value)
        proc_history.emplace_back(new ThresholdActionCtx(DUOTONE,
            split_value));

    ++proc_history.back()->count;

    history_ctx.add(*proc_history.back().get());
}

void MainWindow::duotone_cancel()
{
    duotone_visibility_ctx.end_preview();

    ((Duotone*)processors[DUOTONE].get())->clear_preview();
    draw_image();
}

void MainWindow::fill_holes()
{
    PsdData& img = psd_manager.get_image();

    if (img.n_channels != 1)
    {
        QToolTip::showText(
            ui->button_fill->mapToGlobal(QPoint(0, 0)),
            "A duotone 1 channel image is expected for this action",
            ui->button_fill);
        return;
    }

    Fill* fill = (Fill*)processors[FILL].get();

    fill->set_color(0);
    if (!fill->process(img.get_raw()))
        return;

    draw_image();

    if (!proc_history.size() || proc_history.back()->type != FILL)
        proc_history.emplace_back(new ProcCtx(FILL));
    ++proc_history.back()->count;

    history_ctx.add(*proc_history.back().get());
}

void MainWindow::thin_letter(BorderSide side)
{
    PsdData& img = psd_manager.get_image();

    if (img.n_channels != 1)
    {
        QToolTip::showText(
            ui->button_fill->mapToGlobal(QPoint(0, 0)),
            "A duotone 1 channel image is expected for this action",
            ui->button_fill);
        return;
    }

    ProcessorType type = ui->radioButton_thin->isChecked() ?
        THIN : IRREG_CLEANUP;

    DirectionalPrcessor* proc = (DirectionalPrcessor*)processors[type].get();

    proc->set_side(side);
    if (!proc->process(img.get_raw()))
        return;

    draw_image();

    if (!proc_history.size() || proc_history.back()->type != type ||
        ((DirectionalActionCtx*)proc_history.back().get())->side != side)
        proc_history.emplace_back(new DirectionalActionCtx(type, side));
    ++proc_history.back()->count;

    history_ctx.add(*proc_history.back().get());
}

void MainWindow::thin_top()
{
    thin_letter(BorderSide::TOP);
}

void MainWindow::thin_right()
{
    thin_letter(BorderSide::RIGHT);
}

void MainWindow::thin_bottom()
{
    thin_letter(BorderSide::BOTTOM);
}

void MainWindow::thin_left()
{
    thin_letter(BorderSide::LEFT);
}

void MainWindow::trace_letters()
{
    PsdData& img = psd_manager.get_image();

    if (img.n_channels != 1)
    {
        QToolTip::showText(
            ui->button_fill->mapToGlobal(QPoint(0, 0)),
            "A duotone 1 channel image is expected for this action",
            ui->button_fill);
        return;
    }

    LetterFinder* proc = (LetterFinder*)processors[TRACE_LETTERS].get();
    clear_letter_meta();
    size_t last_size = 0;
    auto& letters = proc->get_letters();

    while (proc->process(img.get_raw()))
    {
        proc->process(img.get_raw());

        if (last_size == letters.size())
            continue;

        last_size = letters.size();
        const LetterData& l = proc->last_letter();

        add_letter_meta(l);
    }

    proc->clear();

    QMessageBox::information(this, tr("Letters tracing"),
        tr("Letters tracing is complete"));
}

void MainWindow::import_history()
{
    QString file_name = QFileDialog::getOpenFileName(this,
        tr("Import image preprocessing"), "", tr("Image Pre-Processing (*.ipp)"));
    if (file_name.isEmpty())
        return;

    FILE* file = fopen(file_name.toLocal8Bit().data(), "rb");
    if (!file)
    {
        QMessageBox::warning(this, tr("Error opening file"),
            tr("An error occured while opening selected file."),
            QMessageBox::Ok);
        return;
    }

    size_t size = 0;
    if (!fread(&size, sizeof(size), 1, file))
    {
        QMessageBox::warning(this, tr("Error reading from file"),
            tr("An error occured while reading from selected file."),
            QMessageBox::Ok);
        fclose(file);
        return;
    }

    if (!size)
    {
        QMessageBox::information(this, tr("No action in the file"),
            tr("Selected file doesn't contain any preprocessing actions."));
        fclose(file);
        return;
    }

    proc_history.clear();
    history_ctx.clear();

    for (unsigned i = 0; i < size; ++i)
    {
        ProcCtx ctx = ProcCtx::deserialize(file);

        switch (ctx.type)
        {
        case DUOTONE:
            proc_history.emplace_back(new ThresholdActionCtx(ctx.type, file));
            break;
        case FILL:
            proc_history.emplace_back(new ProcCtx(ctx.type));
            break;
        case THIN: // fallthrough
        case IRREG_CLEANUP:
            proc_history.emplace_back(new DirectionalActionCtx(ctx.type, file));
            break;
        default:
            QMessageBox::warning(this, tr("Error reading from file"),
                tr("File contains unknown actions."),
                QMessageBox::Ok);
            fclose(file);
            return;
        }

        proc_history.back()->count = ctx.count;
        history_ctx.add(*proc_history.back().get());
    }

    fclose(file);

    reapply_history();
}

void MainWindow::export_history()
{
    QString file_name = QFileDialog::getSaveFileName(this,
        tr("Export image preprocessing"), "", tr("Image Pre-Processing (*.ipp)"));
    if (file_name.isEmpty())
        return;

    FILE* file = fopen(file_name.toLocal8Bit().data(), "wb");
    if (!file)
    {
        QMessageBox::warning(this, tr("Error opening file"),
            tr("An error occured while opening selected file."),
            QMessageBox::Ok);
        return;
    }

    auto size = proc_history.size();
    fwrite(&size, sizeof(size), 1, file);

    for (auto& action : proc_history)
        action->serialize(file);

    fclose(file);
}

void MainWindow::reapply_history()
{
    // TODO: would be way easier, if processing actions implemented
    // "command" pattern

    // reopen image to reset current processing
    if (!psd_manager.open(psd_manager.get_path()))
    {
        QMessageBox::warning(this, tr("Error opening file"),
            tr("An error occured while reopening image file."),
            QMessageBox::Ok);
        return;
    }
    clear_letter_meta();

    if (psd_manager.get_image().n_channels > 1)
        processors[GRAYSCALE]->process(psd_manager.get_image().get_raw());

    for (auto& act : proc_history)
    {

        switch (act->type)
        {
        case DUOTONE:
        {
            Duotone* duotone = (Duotone*)processors[DUOTONE].get();
            auto ctx = (ThresholdActionCtx*)act.get();
            duotone->set_split_value(ctx->threshold);

            duotone->process(psd_manager.get_image().get_raw());

            psd_manager.get_image().get_raw() = duotone->get_preview();
            duotone->clear_preview();
        }
            break;
        case FILL:
        {
            Fill* fill = (Fill*)processors[FILL].get();

            fill->set_color(0);
            fill->process(psd_manager.get_image().get_raw());
        }
            break;
        case THIN: // fallthrough
        case IRREG_CLEANUP:
        {
            auto ctx = (DirectionalActionCtx*)act.get();
            auto proc = (DirectionalPrcessor*)processors[ctx->type].get();

            proc->set_side(ctx->side);
            proc->process(psd_manager.get_image().get_raw());
        }
            break;
        default:
            QMessageBox::warning(this, tr("Error applying history"),
                tr("An error occured while applying history from the file."),
                QMessageBox::Ok);
            // TODO: what should happen here?
            return;
        }
    }

    draw_image();
}

// utility impl
namespace ui_context {
    static void side_to_str(BorderSide side, std::stringstream& ss)
    {
        switch (side) {
        case BorderSide::TOP:
            ss << "top";
            break;
        case BorderSide::RIGHT:
            ss << "right";
            break;
        case BorderSide::BOTTOM:
            ss << "bottom";
            break;
        case BorderSide::LEFT:
            ss << "left";
            break;
        default:
            break;
        }
    }

    static void proc_to_str(const ProcCtx* ctx, std::stringstream& ss)
    {
        switch (ctx->type)
        {
        case DUOTONE:
            ss << "Duotone, " << ((ThresholdActionCtx*)ctx)->threshold;
            break;
        case FILL:
            ss << "Fill holes";
            break;
        case THIN:
            ss << "Thin, ";
            side_to_str(((DirectionalActionCtx*)ctx)->side, ss);
            break;
        case IRREG_CLEANUP:
            ss << "Cleanup, ";
            side_to_str(((DirectionalActionCtx*)ctx)->side, ss);
            break;
        default:
            break;
        }
    }
}

void ProcHistoryManager::add(const ProcCtx& ctx)
{
    std::stringstream ss;

    proc_to_str(&ctx, ss);

    if (ctx.count > 1)
    {
        ss << ", " << ctx.count << " times";
        list->removeLast();
    }

    list->append(ss.str().c_str());
    model->setStringList(*list);
}

void ProcHistoryManager::clear()
{
    list->clear();
    model->setStringList(*list);
}

// Letters info
const QPen LetterRect::red = QPen(Qt::red);
const QPen LetterRect::green = QPen(Qt::green);

static QString metrics_to_str(const std::vector<int> vec)
{
    if (!vec.size())
        return "0";

    QString res;

    res.append(std::to_string(vec[0]).c_str());

    for (int i = 1; i < vec.size(); ++i)
    {
        res += ", ";
        res += std::to_string(vec[i]).c_str();
    }

    return res;
}

void LetterInfoCtx::apply_info(const LetterData& letter)
{
    label_hor_cnt->setText(metrics_to_str(letter.horizontal_lines));
    label_vert_cnt->setText(metrics_to_str(letter.vertical_lines));
}

void LetterInfoCtx::clear_info()
{
    label_hor_cnt->setText("0");
    label_vert_cnt->setText("0");
    last_pressed = nullptr;
}

LetterRect::LetterRect(const class LetterData& letter)
    : letter(letter), pen(red)
{}

QRectF LetterRect::boundingRect() const
{
    return QRectF(letter.top_left.x, letter.top_left.y,
        letter.width(), letter.height());
}

void LetterRect::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    QRectF rect(letter.top_left.x - 1, letter.top_left.y - 1,
        letter.width() + 2, letter.height() + 2);

    painter->setPen(pen);
    painter->drawRect(rect);
}

void LetterRect::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
    pen = green;

    if (letter_info_ctx.last_pressed && letter_info_ctx.last_pressed != this)
        letter_info_ctx.last_pressed->pen = red;

    letter_info_ctx.last_pressed = this;
    letter_info_ctx.apply_info(letter);

    update();
}
