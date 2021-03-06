#include "main_window.h"
#include "src/gui/filter_results_table.h"
#include "src/gui/timer_results_table.h"
#include <QTabWidget>
#include <QVBoxLayout>

#include "src/helpers/log_in_file_manager.h"

#include "src/helpers/alert_helper.h"

#include <pthread.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_tablesIsVisible(false)
{
    loadFonts();
    initControls();
    initLayouts();
    initStatusBar();

    QRect screenRect = QApplication::desktop()->availableGeometry();
    setGeometry(screenRect.x() + 25, screenRect.y() + 50, this->minimumWidth(), screenRect.height() - 75);

    initGraphWindow();
}

MainWindow::~MainWindow()
{
    delete m_graphWindow;
}

void MainWindow::loadFonts()
{
    this->setFont(FontManager::instance().regular(GuiConfig::FONT_SIZE_NORMAL));

    if (!statusBar()) {
        setStatusBar(new QStatusBar);
    }
    this->statusBar()->setFont(FontManager::instance().regularItalic(GuiConfig::FONT_SIZE_SMALL));
}

void MainWindow::initControls()
{
    m_taskWidget               = new TaskWidget;
    m_filterParamsWidget       = new FilterParametersWidget;
    m_filterStartWidget        = new FilterStartButtonsBox;
    m_pinvMethodSettingsWidget = new PinvMethodSettingWidget;
    m_startConditionsFilterWidget = new StartConditionsFilterWidget;
    m_btnClear                 = new QPushButton(tr("Очистить"));
    m_btnShowHideTables        = new QPushButton(tr("Показать таблицы"));
    m_btnShowTimes             = new QPushButton(tr("Показать время выполнения"));

    m_btnShowHideTables->setEnabled(false);
    m_btnShowTimes->setEnabled(false);

    connect(m_btnClear, SIGNAL(clicked()), this, SIGNAL(clear()));
    connect(m_btnShowHideTables, SIGNAL(clicked()), this, SLOT(onShowHideTables()));
    connect(m_btnShowTimes, SIGNAL(clicked()), this, SLOT(onShowTableTimer()));
    connect(m_taskWidget, SIGNAL(changed()), this, SIGNAL(clear()));
    connect(this, SIGNAL(clear()), this, SLOT(onClear()));
    connect(m_filterStartWidget, SIGNAL(start(Core::FILTER_TYPE, Core::APPROX_TYPE, FILTER_ID)), this,
            SLOT(onStart(Core::FILTER_TYPE, Core::APPROX_TYPE, FILTER_ID)));
    connect(m_filterStartWidget, SIGNAL(filtersFamilyChanged(Core::FILTER_TYPE)), m_filterParamsWidget,
            SLOT(onFiltersFamilyChanged(Core::FILTER_TYPE)));
    connect(m_filterStartWidget, SIGNAL(filtersFamilyChanged(Core::FILTER_TYPE)), m_taskWidget,
            SLOT(onFiltersFamilyChanged(Core::FILTER_TYPE)));
}

void MainWindow::initLayouts()
{
    // page1:

    QVBoxLayout *page1Layout = new QVBoxLayout;
    page1Layout->setMargin(GuiConfig::LAYOUT_MARGIN_BIG);
    page1Layout->setSpacing(GuiConfig::LAYOUT_SPACING_BIG);
    page1Layout->addWidget(m_taskWidget);
    page1Layout->addWidget(m_filterParamsWidget);
    page1Layout->addWidget(m_filterStartWidget);
    page1Layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    QHBoxLayout *btnLayout1 = new QHBoxLayout;
    btnLayout1->setMargin(GuiConfig::LAYOUT_MARGIN_NORMAL);
    btnLayout1->setSpacing(GuiConfig::LAYOUT_SPACING_NORMAL);
    btnLayout1->addWidget(m_btnClear);
    btnLayout1->addWidget(m_btnShowHideTables);

    QHBoxLayout *btnLayout2 = new QHBoxLayout;
    btnLayout2->setMargin(GuiConfig::LAYOUT_MARGIN_NORMAL);
    btnLayout2->setSpacing(GuiConfig::LAYOUT_SPACING_NORMAL);
    btnLayout2->addWidget(m_btnShowTimes);

    page1Layout->addLayout(btnLayout1);
    page1Layout->addLayout(btnLayout2);

    QWidget *page1 = new QWidget;
    page1->setLayout(page1Layout);
    page1->setMinimumWidth(page1Layout->margin() * 2 + m_filterParamsWidget->minimumWidth());

    // page2:

    QVBoxLayout *page2Layout = new QVBoxLayout;
    page2Layout->setMargin(GuiConfig::LAYOUT_MARGIN_BIG);
    page2Layout->setSpacing(GuiConfig::LAYOUT_SPACING_BIG);
    page2Layout->addWidget(m_pinvMethodSettingsWidget);
    page2Layout->addWidget(m_startConditionsFilterWidget);
    page2Layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    QWidget *page2 = new QWidget;
    page2->setLayout(page2Layout);

    QTabWidget *tabs = new QTabWidget;
    tabs->addTab(page1, tr("Основное"));
    tabs->addTab(page2, tr("Дополнительные настройки"));

    QWidget *    mainWidget = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(GuiConfig::LAYOUT_MARGIN_SMALL);
    mainLayout->setSpacing(GuiConfig::LAYOUT_SPACING_SMALL);
    mainLayout->addWidget(tabs);
    mainWidget->setLayout(mainLayout);

    setCentralWidget(mainWidget);

    centralWidget()->setMinimumWidth(2 * GuiConfig::LAYOUT_MARGIN_BIG + page1->minimumWidth());
    setMinimumWidth(this->layout()->margin() * 2 + centralWidget()->minimumWidth());
}

void MainWindow::initStatusBar()
{
    int w = centralWidget()->minimumWidth() / 3;
    int h = QFontMetrics(statusBar()->font()).height();

    m_statusProgressBar = new QProgressBar;
    m_statusProgressBar->setMinimumWidth(w);
    m_statusProgressBar->setMaximumWidth(w);
    m_statusProgressBar->setMinimumHeight(h);
    m_statusProgressBar->setMaximumHeight(h);
    m_statusProgressBar->setEnabled(false);
    m_statusProgressBar->setValue(0);

    statusBar()->addPermanentWidget(m_statusProgressBar);
    statusBar()->layout()->setMargin(centralWidget()->layout()->margin());
    statusBar()->layout()->setSpacing(centralWidget()->layout()->spacing());
    statusBar()->showMessage(tr("Состояние: ничего не выполняется"));
}

void MainWindow::initGraphWindow()
{
    m_graphWindow = new GraphWindow;
    m_graphWindow->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint |
                                  Qt::CustomizeWindowHint);
    m_graphWindow->setWindowTitle(tr("Графики"));
    QRect screenRect = QApplication::desktop()->availableGeometry();
    m_graphWindow->setGeometry(this->x() + this->width() + 25, screenRect.y() + 50, screenRect.width() - this->width() - 75, screenRect.height() - 75);

    connect(this, SIGNAL(clear()), m_graphWindow, SLOT(onClear()));

    m_graphWindow->updatePlotter();
    m_graphWindow->show();
}

void MainWindow::closeEvent(QCloseEvent *)
{
    QApplication::closeAllWindows();
}


// slots:

void MainWindow::onClear()
{
    m_colorManager.reset();
    m_filter_time_results.clear();

    for (int i = 0; i < m_tables.size(); ++i) {
        m_tables[i]->close();
        delete m_tables[i];
    }
    m_tables.clear();

    m_tablesIsVisible = false;
    m_btnShowHideTables->setText(tr("Показать таблицы"));
    m_btnShowHideTables->setEnabled(false);
    m_btnShowTimes->setEnabled(false);
}

void MainWindow::onShowHideTables()
{
    if (m_tables.size() == 0) {
        return;
    }

    if (m_tablesIsVisible) {
        for (int i = 0; i < m_tables.size(); ++i) {
            m_tables[i]->hide();
        }
        m_tablesIsVisible = false;
        m_btnShowHideTables->setText(tr("Показать таблицы"));
    } else {
        for (int i = 0; i < m_tables.size(); ++i) {
            m_tables[i]->show();
        }
        m_tablesIsVisible = true;
        m_btnShowHideTables->setText(tr("Скрыть таблицы"));
    }
}

void MainWindow::onShowTableTimer()
{
    if (m_filter_time_results.count() > 0) {
        TimerResultsTable *timerTable = new TimerResultsTable(m_filter_time_results);
        timerTable->show();
    }
}

void MainWindow::onFilterUpdatePercent(int p)
{
    m_statusProgressBar->setValue(p);
}

void MainWindow::onStart(Core::FILTER_TYPE ftype, Core::APPROX_TYPE /*atype*/, FILTER_ID id)
{
    bool normalTask = m_taskWidget->taskIsNull(ftype);
    if (!normalTask) {
        AlertHelper::showErrorAlertWithText("Выбран не верный тип фильтров для данной задачи");
        return;
    }
    if (m_runningFilters.count() > 0) {
        AlertHelper::showWarningAlertWithText("Дождитесь завершения моделирования предыдущего фильтра");
        return;
    }
    RunningFilter runningFilter;
    runningFilter.task = m_taskWidget->task(ftype);
    runningFilter.ftype = ftype;

    Core::PtrFilterParameters params = m_filterParamsWidget->parameters();
    params->setInitialCondition(m_startConditionsFilterWidget->initialConditionForFilter());

    runningFilter.filter = Filters::FilterFactory::create(ftype, id, params, runningFilter.task);

    QThread *thread = new QThread;
    runningFilter.filter->moveToThread(thread);

    m_runningFilters.append(runningFilter);

    connect(runningFilter.filter.get(), SIGNAL(updatePercent(int)), this, SLOT(onFilterUpdatePercent(int)));

    connect(thread, SIGNAL(started()), runningFilter.filter.get(), SLOT(run()));
    connect(runningFilter.filter.get(), SIGNAL(filterFinishExecute()), thread, SLOT(quit()));
    connect(thread, SIGNAL(finished()), this, SLOT(onFinishExecutingFilter()));

    m_statusProgressBar->setEnabled(true);
    QString status = tr("Состояние: выполняется ") + QString::fromStdString(runningFilter.filter->info()->full_name());
    statusBar()->showMessage(status);

    thread->start();
}

void MainWindow::onFinishExecutingFilter()
{
    RunningFilter runningFilter = m_runningFilters.first();

    showData(runningFilter.filter, runningFilter.ftype, runningFilter.task);
    m_filter_time_results.append(runningFilter.filter->execute_time());
    this->statusBar()->showMessage(tr("Состояние: ничего не выполняется"));
    m_statusProgressBar->setEnabled(false);
    m_statusProgressBar->setValue(0);
    m_runningFilters.clear();
    m_btnShowTimes->setEnabled(true);
}

void MainWindow::showData(Core::PtrFilter filter, Core::FILTER_TYPE ftype, Core::PtrTask task)
{
    int colorIndex = m_colorManager.nextColorIndex();
    QColor color = m_colorManager.nextColorAtIndex(colorIndex);
//    QColor color_realization_e = m_colorManager.nextColorAtIndex(colorIndex);
//    QColor color_realization_z = m_colorManager.nextColorAtIndex(colorIndex);
    QColor color_realization_e = m_colorManager.nextColorRealizationE();
    QColor color_realization_z = m_colorManager.nextColorRealizationZ();
//    QColor color_PI = m_colorManager.nextColorPI();
    QString fname = QString::fromStdString(filter->info()->full_name()) + tr("; ");

    QPen mxPen, mePen, sxPen, sePen, upDownX, upDownE, selectRealizX, selectRealizE, selectRealizZ, deltaIColor, PIColor, PdeltaIColor;
    mxPen.setWidthF(2.0);
    mxPen.setColor(Qt::darkMagenta);
    mePen.setWidthF(2.0);
    mePen.setColor(color);
    mePen.setStyle(Qt::DashDotLine);
    sxPen.setWidthF(1.5);
    sxPen.setColor(Qt::darkGray);
    sxPen.setStyle(Qt::DashLine);
    sePen.setWidthF(1.5);
    sePen.setColor(color);
    upDownX.setWidthF(1.0);
    upDownX.setColor(Qt::magenta);
    upDownE.setWidthF(1.0);
    upDownE.setColor(color_realization_e);
    selectRealizX.setWidthF(1.5);
    selectRealizX.setColor(Qt::darkGreen);
    selectRealizE.setWidthF(1.5);
    selectRealizE.setColor(color_realization_e);
    selectRealizZ.setWidthF(1.5);
    selectRealizZ.setColor(color_realization_z);
    deltaIColor.setWidthF(1.5);
    deltaIColor.setColor(color_realization_z);
    PIColor.setWidthF(1.5);
    PIColor.setColor(Qt::darkMagenta);
    PdeltaIColor.setWidthF(1.5);
    PdeltaIColor.setColor(color);

    int dim = int(filter->result()[0].meanX.size());
    if (m_graphWindow->countSheets() != dim) {
        m_graphWindow->setCountSheets(dim);
        m_graphWindow->setCountRealizationSheets(dim);
        m_graphWindow->setCountSeToSxSheets(dim);
    }

    QString titleStatistic = tr("Статистика <") + m_taskWidget->name() + QString(">");
    QString titleRealization = tr("Реализации <") + m_taskWidget->name() + QString(">");
    QString titleParametrs = tr("Параметры <") + m_taskWidget->name() + QString(">");
    QString titleSeToSx = tr("Se / Sx <") + m_taskWidget->name() + QString(">");
    QString subTitle = subtitleForParametrs(ftype, task);
    for (int i = 0; i < dim; i++) {
        if (i < dim) {
            m_graphWindow->sheetAtIndex(i).setTitleLabel(titleStatistic);
            m_graphWindow->sheetAtIndex(i).setSubTitleLabel(subTitle);
            m_graphWindow->realizationSheetAtIndex(i).setTitleLabel(titleRealization);
            m_graphWindow->realizationSheetAtIndex(i).setSubTitleLabel(subTitle);
            m_graphWindow->SeToSxSheetAtIndex(i).setTitleLabel(titleSeToSx);
            m_graphWindow->SeToSxSheetAtIndex(i).setSubTitleLabel(subTitle);
        }
    }

    TASK_ID taskId = m_taskWidget->id();
    if (taskId == LandingLinear || taskId == LandingGauss ||taskId == LDLandingRejection3DLinear  || taskId == LDLandingRejection6DLinear) {
        m_graphWindow->sheetAtIndex(0).setXLabel(tr("Время (с)"));
        m_graphWindow->sheetAtIndex(1).setXLabel(tr("Время (с)"));
        m_graphWindow->sheetAtIndex(2).setXLabel(tr("Время (с)"));
        m_graphWindow->sheetAtIndex(0).setYLabel(tr("По скорости (м/c)"));
        m_graphWindow->sheetAtIndex(1).setYLabel(tr("По уголу наклона (°)"));
        m_graphWindow->sheetAtIndex(2).setYLabel(tr("По высоте (м)"));

        m_graphWindow->realizationSheetAtIndex(0).setXLabel(tr("Время (с)"));
        m_graphWindow->realizationSheetAtIndex(1).setXLabel(tr("Время (с)"));
        m_graphWindow->realizationSheetAtIndex(2).setXLabel(tr("Время (с)"));
        m_graphWindow->realizationSheetAtIndex(0).setYLabel(tr("По скорости (м/с)"));
        m_graphWindow->realizationSheetAtIndex(1).setYLabel(tr("ПО уголу наклона (°)"));
        m_graphWindow->realizationSheetAtIndex(2).setYLabel(tr("По высоте (м)"));

        m_graphWindow->SeToSxSheetAtIndex(0).setXLabel(tr("Время (с)"));
        m_graphWindow->SeToSxSheetAtIndex(1).setXLabel(tr("Время (с)"));
        m_graphWindow->SeToSxSheetAtIndex(2).setXLabel(tr("Время (с)"));
        m_graphWindow->SeToSxSheetAtIndex(0).setYLabel(tr("По скорости (м/c)"));
        m_graphWindow->SeToSxSheetAtIndex(1).setYLabel(tr("По уголу наклона (°)"));
        m_graphWindow->SeToSxSheetAtIndex(2).setYLabel(tr("По высоте (м)"));
    }

    if (taskId == LDScalarRejectionLinear) {
        m_graphWindow->sheetAtIndex(0).setXLabel(tr("Время (с)"));
    }
    if (taskId == LDLandingRejection6DLinear) { // продолжаем 4, 5, 6 координаты потому что первый if уже сработал как надо
        m_graphWindow->sheetAtIndex(3).setXLabel(tr("Время (с)"));
        m_graphWindow->sheetAtIndex(4).setXLabel(tr("Время (с)"));
        m_graphWindow->sheetAtIndex(5).setXLabel(tr("Время (с)"));
        m_graphWindow->sheetAtIndex(3).setYLabel(tr("По квазиплотности (1/м)"));
        m_graphWindow->sheetAtIndex(4).setYLabel(tr("По качеству"));
        m_graphWindow->sheetAtIndex(5).setYLabel(tr("По ошибке гировертикали (°)"));

        m_graphWindow->realizationSheetAtIndex(3).setXLabel(tr("Время (с)"));
        m_graphWindow->realizationSheetAtIndex(4).setXLabel(tr("Время (с)"));
        m_graphWindow->realizationSheetAtIndex(5).setXLabel(tr("Время (с)"));
        m_graphWindow->realizationSheetAtIndex(3).setYLabel(tr("Квазиплотности (1/м)"));
        m_graphWindow->realizationSheetAtIndex(4).setYLabel(tr("Качества"));
        m_graphWindow->realizationSheetAtIndex(5).setYLabel(tr("Ошибки гировертикали (°)"));

        m_graphWindow->SeToSxSheetAtIndex(3).setXLabel(tr("Время (с)"));
        m_graphWindow->SeToSxSheetAtIndex(4).setXLabel(tr("Время (с)"));
        m_graphWindow->SeToSxSheetAtIndex(5).setXLabel(tr("Время (с)"));
        m_graphWindow->SeToSxSheetAtIndex(3).setYLabel(tr("По квазиплотности (1/м)"));
        m_graphWindow->SeToSxSheetAtIndex(4).setYLabel(tr("По качеству"));
        m_graphWindow->SeToSxSheetAtIndex(5).setYLabel(tr("По ошибке гировертикали (°)"));
    }
    Math::Vector scale(dim);
    if (taskId == LDLandingRejection3DLinear) {
        scale[0] = 1000.0;
        scale[2] = 1.0;
        scale[1] = Math::Convert::RadToDeg(1.0);
    } else if (taskId == LDLandingRejection6DLinear) {
        scale[0] = 1000.0;
        scale[2] = 1.0;
        scale[1] = scale[5] = Math::Convert::RadToDeg(1.0);
        scale[3] = 1000.0;
        scale[4] = 1000.0;
    } else if (taskId == LandingLinear || taskId == LandingGauss) {
        scale[0] = 1000.0;
        scale[1] = Math::Convert::RadToDeg(1.0);
        scale[2] = 1.0;
    } else {
        for (int i = 0; i < dim; ++i) {
            scale[i] = 1.0;
        }
    }

    QVector<double> x, y, sx, seToSx, y_up, y_down;
    Core::GetTime(filter->result(), x);

    int numberTraectory = filter->params()->specificRealization();

    for (int i = 0; i < dim; i++) {
        Core::GetMeanX(filter->result(), i, y, scale[i]);
        m_graphWindow->sheetAtIndex(i).addCurve(x, y, "Mx" + QString::number(i + 1), mxPen, false);

        Core::GetStdDeviationX(filter->result(), i, sx, scale[i]);
        m_graphWindow->sheetAtIndex(i).addCurve(x, sx, "Sx" + QString::number(i + 1), sxPen, false);

        Core::GetMeanE(filter->result(), i, y, scale[i]);
        m_graphWindow->sheetAtIndex(i).addCurve(x, y, "Me" + QString::number(i + 1) + " " + fname, mePen, false);

        Core::GetStdDeviationE(filter->result(), i, y, scale[i]);
        double avrg = 0;
        for (auto se : y) {
            avrg += se / y.size();
        }
        m_graphWindow->sheetAtIndex(i).addCurve(x, y, "Se" + QString::number(i + 1) + " " + fname + "среднее = " + QString::number(avrg), sePen, true);

        seToSx.resize(y.size());

        double avrgSeToSx = 0;
        for (int index = 0; index < y.size(); index++) {
            seToSx[index] = y[index] / sx[index];
            avrgSeToSx += seToSx[index] / y.size();
        }

        m_graphWindow->SeToSxSheetAtIndex(i).addCurve(x, seToSx, "Se" + QString::number(i + 1) + "/Sx" + QString::number(i + 1) + " " + fname + "среднее = " + QString::number(avrgSeToSx), sePen, true);

        Core::GetRealizationE(filter->result(), i, y, scale[i]);
        Core::GetUpE(filter->result(), i, y_up, scale[i]);
        Core::GetDownE(filter->result(), i, y_down, scale[i]);
        QString name_sheet_e = "E" + QString::number(i + 1) + " (" + QString::number(numberTraectory) + ") " + fname;
        m_graphWindow->realizationSheetAtIndex(i).addCurve(x, y, y_up, y_down, name_sheet_e, selectRealizE, upDownE, true);

        Core::GetRealizationZ(filter->result(), i, y, scale[i]);
        QString name_sheet_z = "Z" + QString::number(i + 1) + " (" + QString::number(numberTraectory) + ") " + fname;
        m_graphWindow->realizationSheetAtIndex(i).addCurve(x, y, name_sheet_z, selectRealizZ, false);

        Core::GetRealizationX(filter->result(), i, y, scale[i]);
//        Core::GetUpX(filter->result(), i, y_up, scale[i]);
//        Core::GetDownX(filter->result(), i, y_down, scale[i]);
//        m_graphWindow->sheet(i).addCurve(x, y, y_up, y_down, name_sheet_x, selectRealizX, upDownX);
        QString name_sheet_x = "X" + QString::number(i + 1) + " (" + QString::number(numberTraectory) + ") ";
        m_graphWindow->realizationSheetAtIndex(i).addCurve(x, y, name_sheet_x, selectRealizX, false);
    }

//    for (auto param : filter->m_specific_params) {
//        Core::GetStdDeviationX(filter->result(), i, y, scale[i]);
//        m_graphWindow->sheetAtIndex(i).addCurve(x, y, "Sx" + QString::number(i + 1), sxPen, false);
//    }

    if (ftype == LogicDynamic) {
        QVector<double> I, evaluationI, deltaI, PI, PdeltaI;
        double scaleI = 1.0;

        Core::GetI(filter->result(), numberTraectory, I, scaleI);
        Core::GetEvaluationI(filter->result(), numberTraectory, evaluationI, scaleI);
        Core::GetDeltaI(filter->result(), numberTraectory, deltaI, scaleI);

        Core::GetPI(filter->result(), PI, scaleI);
        Core::GetPDeltaI(filter->result(), PdeltaI, scaleI);

        GAxisRange customRange;
        customRange.xMin = 0.0;
        customRange.xMax = *std::max_element(x.begin(), x.end());
        customRange.yMin = -0.2;
        customRange.yMax = 1.2;

        if (m_graphWindow->reloadStatisticSheet()) {
            m_graphWindow->statisticLDSheet().setTitleLabel(titleStatistic);
            m_graphWindow->statisticLDSheet().setSubTitleLabel(subTitle);

            m_graphWindow->statisticLDSheet().setXLabel(tr("Время (с)"));
            m_graphWindow->statisticLDSheet().setYLabel(tr("Вероятность режима"));

            m_graphWindow->statisticLDSheet().addICurve(x, PI, tr("P (I = 1)"), PIColor, customRange, true);
        }

        m_graphWindow->statisticLDSheet().addICurve(x, PdeltaI, tr("P (ΔI = 0)") + " " + fname, PdeltaIColor, customRange, true);

        customRange.yMin = -0.5;
        customRange.yMax = *std::max_element(I.begin(), I.end()) + 0.5;

        if (m_graphWindow->reloadRealizationSheet()) {
            m_graphWindow->realizationLDSheet().setTitleLabel(titleRealization);
            m_graphWindow->realizationLDSheet().setSubTitleLabel(subTitle);

            m_graphWindow->realizationLDSheet().setXLabel(tr("Время (с)"));
            m_graphWindow->realizationLDSheet().setYLabel(tr("Номер режима"));

            m_graphWindow->realizationLDSheet().addICurve(x, I, tr("I") + " (" + QString::number(numberTraectory) + ") ", mxPen, customRange, false);
        }

        m_graphWindow->realizationLDSheet().addICurve(x, evaluationI, tr("Оценка I") + " (" + QString::number(numberTraectory) + ") " + " " + fname, selectRealizE, customRange, false);
        m_graphWindow->realizationLDSheet().addICurve(x, deltaI, tr("ΔI") + " (" + QString::number(numberTraectory) + ") " + " " + fname, deltaIColor, customRange, true);

    }

    if (filter->m_specific_params.size() > 0) {
        if (m_graphWindow->reloadFilterParamsSheet()) {
            m_graphWindow->parametersSheet().setTitleLabel(titleParametrs);
            m_graphWindow->parametersSheet().setSubTitleLabel(tr(" "));

            m_graphWindow->parametersSheet().setXLabel(tr("Время (с)"));
            m_graphWindow->parametersSheet().setYLabel(tr("Значение параметра"));

            for (auto param : filter->m_specific_params) {

                QPen paramPen;
                paramPen.setWidthF(1.5);
                paramPen.setColor(m_colorManager.nextColorAtIndex(m_colorManager.nextColorIndex()));

                m_graphWindow->parametersSheet().addCurve(x, param.second, QString::fromStdString(param.first) + " " + fname, paramPen, true, true);
            }
        }
    }

    m_graphWindow->updateDefaultSheet();
    m_graphWindow->updatePlotter();
    std::string filter_name = filter->info()->name();
    addTable(filter->result(), filter_name, scale);
}

QString MainWindow::subtitleForParametrs(Core::FILTER_TYPE ftype, Core::PtrTask task) {
    QString ss_filter = tr(", размер выборки ") + QString::number(m_filterParamsWidget->parameters()->sampleSize());
    QString subTitle = m_startConditionsFilterWidget->initialConditionString();
    TASK_ID taskId = m_taskWidget->id();
    if (ftype == Discrete || ftype == LogicDynamic) {
        if (taskId == LDScalarRejectionLinear) {
            subTitle = subTitle + tr(" СКО выброса ") + QString::number(task->params()->at("с(2)"));
        } else if (taskId == LDLandingRejection3DLinear || taskId == LDLandingRejection6DLinear) {
            subTitle = subTitle + tr("шаг между измерениями ") + QString::number(m_filterParamsWidget->parameters()->measurementStep());
        } else {
          subTitle = subTitle +
            tr("шаг между измерениями ") + QString::number(m_filterParamsWidget->parameters()->measurementStep());
        }
    } else {
        subTitle = subTitle +
                tr("шаг интегрирования ") + QString::number(m_filterParamsWidget->parameters()->integrationStep()) +
                tr(", шаг между измерениями ") + QString::number(m_filterParamsWidget->parameters()->measurementStep());
    }
    subTitle = subTitle + ss_filter;
    return subTitle;
}

void MainWindow::addTable(const Core::FilterOutput &data, const std::string &label, const Math::Vector &scale)
{
    m_tables.append(new FilterResultsTable(data, label, scale));
    m_tables.last()->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint |
                                    Qt::CustomizeWindowHint);

    QRect screenRect = QApplication::desktop()->availableGeometry();
    int   width      = m_tables.last()->minimumWidth();
    int   height     = m_tables.last()->minimumHeight();

    for (int i = 0; i < m_tables.size(); ++i) {
        m_tables[i]->setGeometry(screenRect.x() + 75 + i * 50, screenRect.y() + 90 + i * 40, width, height);
    }

    if (m_tablesIsVisible) {
        m_tables.last()->show();
    }
    m_btnShowHideTables->setEnabled(true);
}
