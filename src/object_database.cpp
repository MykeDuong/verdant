#include "object_database.h"
#include "object.h"
#include "visitor.h"

ObjectDatabase::ObjectDatabase(std::string identifier) {
  this->type = OBJECT_DATABASE;
  this->identifier = identifier;
}

ObjectDatabase::~ObjectDatabase() {}

void ObjectDatabase::accept(Visitor* visitor) {
  visitor->visit(this);
}
