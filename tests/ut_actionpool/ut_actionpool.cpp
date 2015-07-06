#include <MApplication>
#include <QDebug>

#include <MAction>

#include "actionpool.h"
#include "ut_actionpool.h"

void Ut_ActionPool::testSingelton()
{
    QVERIFY(0 != ActionPool::instance());
}

void Ut_ActionPool::testGetAction()
{
    ActionPool * actions = ActionPool::instance();
    QVERIFY(0 != actions);

    const ActionPool::Id actionIds[] = {
        ActionPool::ShowAllPagesView,
        ActionPool::ShowNormalView,
        ActionPool::TwoThumbsPerColumn,
        ActionPool::ThreeThumbsPerColumn,
        ActionPool::FourThumbsPerColumn,
        ActionPool::FiveThumbsPerColumn,
        ActionPool::ViewDetails,
        ActionPool::Delete,
        ActionPool::SaveAs,
        ActionPool::Find,
        ActionPool::Share,
        ActionPool::ZoomFitToWidth,
        ActionPool::ZoomFitToPage,
        ActionPool::ZoomLastUserDefined,
        ActionPool::Zoom100percent,
        ActionPool::ZoomToogleZoom,
        ActionPool::SpreadSheetFloatingIndicators,
        ActionPool::SpreadSheetFixedIndicators,
        ActionPool::SpreadSheetNoIndicators,
        ActionPool::FindFirst,
        ActionPool::FindPrevious,
        ActionPool::FindNext,
        ActionPool::ContextMenu
    };

    for(unsigned int i = 0; i < (sizeof(actionIds) / sizeof actionIds[0]); i++) {
        QVERIFY(0 != actions->getAction(actionIds[i]));
    }
}

void Ut_ActionPool::testNegativeGetAction()
{
    ActionPool * actions = ActionPool::instance();
    QVERIFY(0 != actions);

    ActionPool::Id id = (ActionPool::Id) 0xffff;

    QVERIFY(0 == actions->getAction(id));
}

int main(int argc, char* argv[])
{
    MApplication app(argc, argv);
    Ut_ActionPool test;
    return QTest::qExec(&test, argc, argv);
}
