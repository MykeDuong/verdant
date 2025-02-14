#include "database_node.hpp"
#include "visitor.hpp"

DatabaseNode::DatabaseNode(const std::string& name) : name(name) {}

const std::string& DatabaseNode::getName() const {
  return this->name;
}

const VerdantObjectType DatabaseNode::getType() const {
  return VerdantObjectType::DATABASE;
}

void DatabaseNode::accept(Visitor* visitor) {
  visitor->visit(this);
}

