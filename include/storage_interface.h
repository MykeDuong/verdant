#pragma once

class StorageInterface {
public:
  virtual void save() = 0;
  virtual void load() = 0;
  virtual bool isLoaded() = 0;
};
