#pragma once

struct Visitor;
struct ASTNode {
  virtual void accept(Visitor* visitor) = 0;
};

