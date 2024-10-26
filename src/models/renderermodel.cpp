#include "renderermodel.h"

RendererModel::RendererModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

int RendererModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return items.size();
}

int RendererModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 2; // Two columns for name and type
}

QModelIndex RendererModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || row >= items.size() || column < 0 || column >= 2 || parent.isValid())
        return QModelIndex();
    return createIndex(row, column);
}

QModelIndex RendererModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QModelIndex(); // No parent for top-level items
}

QVariant RendererModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const Item &item = items.at(index.row());

    switch (role)
    {
    case Qt::DisplayRole:
        return item.name; // Return name for DisplayRole
    case Qt::UserRole:
        return item.type; // Return type for UserRole
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> RendererModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "name"; // Role for name
    roles[Qt::UserRole] = "type";    // Role for type
    return roles;
}

void RendererModel::addItem(const QString &name, const QString &type)
{
    beginInsertRows(QModelIndex(), items.size(), items.size());
    items.append({name, type}); // Create an Item and append it
    endInsertRows();
}

void RendererModel::removeItem(int row)
{
    if (row < 0 || row >= items.size())
        return;
    beginRemoveRows(QModelIndex(), row, row);
    items.removeAt(row);
    endRemoveRows();
}

QList<Item> RendererModel::getItems() const
{
    return items;
}
