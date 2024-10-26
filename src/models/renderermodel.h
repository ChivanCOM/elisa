#ifndef RENDERERMODEL_H
#define RENDERERMODEL_H

#include <QAbstractItemModel>
#include <QStringList>
#include <QString>
#include <QList>
#include <QHash>

struct Item
{
    QString name;
    QString type;
};

class RendererModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit RendererModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addItem(const QString &name, const QString &type);
    void removeItem(int row);
    QList<Item> getItems() const;

private:
    QList<Item> items;
};

#endif // RENDERERMODEL_H
