#include "table_node.h"
#include "visitor.h"

TableNode::TableNode(const std::string& name) : name(name) {}

const std::string& TableNode::getName() const {
  return name;
}

void TableNode::accept(Visitor* visitor) {
  visitor->visit(this);
}

void TableNode::addColumn(const std::string& name, ColumnInfo&& column) {
  columns[name] = column;
}
