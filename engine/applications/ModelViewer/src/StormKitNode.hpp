#pragma once

#include <QtQuick/QSGSimpleTextureNode>
#include <QtQuick/QSGTextureProvider>

QT_BEGIN_NAMESPACE
class QQuickItem;
QT_END_NAMESPACE

class StormKitNode: public QSGTextureProvider, public QSGSimpleTextureNode {
    Q_OBJECT

  public:
    explicit StormKitNode(QQuickItem *item);
    ~StormKitNode() override;

    auto texture() const -> QSGTexture * override;

    auto sync() -> void;

  private Q_SLOTS:
    auto render() -> void;

  private:
    auto initialize() -> void;

    QQuickItem *m_item;
    QSize m_pixelSize;
    qreal m_dpr;

    bool m_initialized = false;

    QSGTexture *m_sgWrapperTexture = nullptr;
};
