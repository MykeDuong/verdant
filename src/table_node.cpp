#include "table_node.hpp"
#include "verdant_object.hpp"
#include "visitor.hpp"

#include <iostream>

TableNode::TableNode(const std::string& name) : name(name) {}

const std::string& TableNode::getName() const {
  return name;
}

const VerdantObjectType TableNode::getType() const {
  return VerdantObjectType::TABLE;
}

void TableNode::accept(Visitor* visitor) {
  visitor->visit(this);
}

bool TableNode::addColumn(std::string name, size_t position, ColumnInfo&& column) {
  if (columns.find(name) != columns.end()) {
    std::cerr << "[ERROR] Duplicate column name" << std::endl;
    return false;
  }
  columns[name] = std::make_pair(position, std::move(column));
  return true;
}
