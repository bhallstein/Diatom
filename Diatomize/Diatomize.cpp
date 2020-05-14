#include "Diatomize.h"

Diatom diatomize(const Diatomize::Descriptor &sd) {
  Diatom d;
  for (auto s : sd.descriptor)
    d[s->getName()] = s->convertToDiatom();
  return d;
}

void antidiatomize(const Diatomize::Descriptor &sd, Diatom &d) {
  for (auto s : sd.descriptor) {
    std::string name = s->getName();
    s->convertFromDiatom(d[name]);
  }
}

