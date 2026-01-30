#pragma once

#include <QtCore/QObject>

#include <QtNodes/NodeData>
#include <QtNodes/NodeDelegateModel>

#include <memory>

using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDelegateModel;
using QtNodes::PortIndex;
using QtNodes::PortType;

/// The class can potentially incapsulate any user data which
/// need to be transferred within the Node Editor graph
class SourceNodeData : public NodeData
{
public:
    NodeDataType type() const override { return NodeDataType{"source", "Image"}; }
};

class algNodeData : public NodeData
{
public:
    NodeDataType type() const override { return NodeDataType{"out", "alg_context"}; }
};

//------------------------------------------------------------------------------

/// The model dictates the number of inputs and outputs for the Node.
/// In this example it has no logic.
class SourceNodeDataModel : public NodeDelegateModel
{
    Q_OBJECT

public:
    virtual ~SourceNodeDataModel() {}

public:
    QString caption() const override { return QString("Naive Data Model"); }

    QString name() const override { return QString("NaiveDataModel"); }

public:
    unsigned int nPorts(PortType const portType) const override
    {
        unsigned int result = 1;

        switch (portType) {
        case PortType::In:
            result = 0;
            break;

        case PortType::Out:
            result = 1;
            break;
        case PortType::None:
            break;
        }

        return result;
    }

    NodeDataType dataType(PortType const portType, PortIndex const portIndex) const override
    {
        switch (portType) {
        case PortType::Out:
            switch (portIndex) {
            case 0:
                return SourceNodeData().type();
            }
            break;

        case PortType::None:
            break;
        }
        // FIXME: control may reach end of non-void function [-Wreturn-type]
        return NodeDataType();
    }

    std::shared_ptr<NodeData> outData(PortIndex const port) override
    {
         return std::make_shared<SourceNodeData>();

    }

    void setInData(std::shared_ptr<NodeData>, PortIndex const) override
    {
        //
    }

    QWidget *embeddedWidget() override { return nullptr; }
};

class AlgNodeDataModel : public NodeDelegateModel
{
    Q_OBJECT

public:
    virtual ~AlgNodeDataModel() {}

public:
    QString caption() const override { return QString("Alg Node"); }

    QString name() const override { return QString("ALG Node"); }

public:
    unsigned int nPorts(PortType const portType) const override
    {
        unsigned int result = 1;

        switch (portType) {
        case PortType::In:
            result = 1;
            break;

        case PortType::Out:
            result = 1;
            break;
        case PortType::None:
            break;
        }

        return result;
    }

    NodeDataType dataType(PortType const portType, PortIndex const portIndex) const override
    {
        switch (portType) {
        case PortType::In:
            switch (portIndex) {
            case 0:
                return SourceNodeData().type();
            }
            break;
        case PortType::Out:
            switch (portIndex) {
            case 0:
                return algNodeData().type();
            }
            break;

        case PortType::None:
            break;
        }
        // FIXME: control may reach end of non-void function [-Wreturn-type]
        return NodeDataType();
    }

    std::shared_ptr<NodeData> outData(PortIndex const port) override
    {
        return std::make_shared<algNodeData>();

    }

    void setInData(std::shared_ptr<NodeData>, PortIndex const) override
    {
        //
    }

    QWidget* embeddedWidget() override { return nullptr; }
};
