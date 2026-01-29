
/*
Fast Flow Vision
*/

//#include "FFVision.h"
//#include <QtWidgets/QApplication>
//
//int main(int argc, char *argv[])
//{
//    QApplication app(argc, argv);
//    FFVision window;
//    window.show();
//    return app.exec();
//}

#include <QtWidgets/QApplication>

#include <QtNodes/ConnectionStyle>
#include <QtNodes/DataFlowGraphModel>
#include <QtNodes/DataFlowGraphicsScene>
#include <QtNodes/GraphicsView>
#include <QtNodes/NodeData>
#include <QtNodes/NodeDelegateModelRegistry>

#include "models.hpp"

using QtNodes::ConnectionStyle;
using QtNodes::DataFlowGraphicsScene;
using QtNodes::DataFlowGraphModel;
using QtNodes::GraphicsView;
using QtNodes::NodeDelegateModelRegistry;


class NumberData : public NodeData
{
public:
    int number = 0;

    NodeDataType type() const override
    {
        return { "number", "Number" };
    }
};

class NumberSourceDataModel : public NaiveDataModel
{
public:
    QString caption() const override { return "Number Source"; }
    bool captionVisible() const override { return true; }
    QString name() const override { return "NumberSource"; }

    unsigned int nPorts(PortType portType) const override
    {
        return portType == PortType::Out ? 1 : 0;
    }

    NodeDataType dataType(PortType, PortIndex) const override
    {
        return { "number", "Number" };
    }

    std::shared_ptr<NodeData> outData(PortIndex) override
    {
        return std::make_shared<NumberData>();
    }

    void setInData(std::shared_ptr<NodeData>, PortIndex) override {}

    QWidget* embeddedWidget() override { return nullptr; }
};

static std::shared_ptr<NodeDelegateModelRegistry> registerDataModels()
{
    auto ret = std::make_shared<NodeDelegateModelRegistry>();

    ret->registerModel<NumberSourceDataModel>("Basic");
    ret->registerModel<NaiveDataModel>("1");
    ret->registerModel<NaiveDataModel>("2");
    ret->registerModel<NaiveDataModel>("3");
    /*
     We could have more models registered.
     All of them become items in the context meny of the scene.

     ret->registerModel<AnotherDataModel>();
     ret->registerModel<OneMoreDataModel>();

   */

    return ret;
}

static void setStyle()
{
    ConnectionStyle::setConnectionStyle(
        R"(
  {
    "ConnectionStyle": {
      "UseDataDefinedColors": true
    }
  }
  )");
}

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    setStyle();

    std::shared_ptr<NodeDelegateModelRegistry> registry = registerDataModels();
    DataFlowGraphModel dataFlowGraphModel(registry);
    dataFlowGraphModel.addNode("sss");
    DataFlowGraphicsScene scene(dataFlowGraphModel);

    GraphicsView view(&scene);

    view.setWindowTitle("Node-based flow editor");
    view.resize(800, 600);
    view.show();

    return app.exec();
}

