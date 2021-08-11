#include "instrumentspanelcontextmenumodel.h"

#include "log.h"
#include "translation.h"

#include "actions/actiontypes.h"

using namespace mu::context;
using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace mu::ui;
using namespace mu::actions;

static const ActionCode SET_ORDER_ACTION("set-order");

InstrumentsPanelContextMenuModel::InstrumentsPanelContextMenuModel(QObject* parent)
    : AbstractMenuModel(parent)
{
}

void InstrumentsPanelContextMenuModel::load()
{
    dispatcher()->reg(this, SET_ORDER_ACTION, this, &InstrumentsPanelContextMenuModel::setOrder);

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        INotationPtr notation = globalContext()->currentNotation();
        m_masterNotation = globalContext()->currentMasterNotation();

        if (!m_masterNotation || !notation || m_masterNotation->notation() != notation) {
            clear();
        } else {
            loadItems();
        }

        emit loaded();
    });
}

void InstrumentsPanelContextMenuModel::loadItems()
{
    TRACEFUNC;

    m_orders.clear();

    RetValCh<InstrumentsMeta> meta = instrumentsRepository()->instrumentsMeta();
    if (!meta.ret) {
        LOGE() << meta.ret.toString();
        return;
    }

    m_orders = meta.val.scoreOrders;

    MenuItemList orderItems;

    for (const ScoreOrder* order : m_orders) {
        MenuItem orderItem;

        orderItem.id = order->id;
        orderItem.title = order->name;
        orderItem.code = SET_ORDER_ACTION;
        orderItem.args = ActionData::make_arg1<QString>(order->id);
        orderItem.checkable = Checkable::Yes;
        orderItem.state.enabled = true;
        orderItem.state.checked = m_masterNotation->notation()->scoreOrder().id == order->id;

        orderItems << orderItem;
    }

    MenuItemList items {
        makeMenu(qtrc("instruments", "Instrument ordering"), orderItems)
    };

    setItems(items);
}

void InstrumentsPanelContextMenuModel::setOrder(const ActionData& args)
{
    QString newOrderId = args.count() > 0 ? args.arg<QString>(0) : QString();
    const ScoreOrder* newOrder = nullptr;

    for (const ScoreOrder* order : m_orders) {
        if (order->id == newOrderId) {
            newOrder = order;
            break;
        }
    }

    if (newOrder) {
        m_masterNotation->parts()->setScoreOrder(*newOrder);
    }
}
