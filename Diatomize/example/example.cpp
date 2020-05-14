//
// example.cpp - Diatomization example/test
//
//   This is an example of serializing a fairly complex object, X.
//   - X even has a property which is a separate class.
//
//   To become Diatomizable, X just needs to provide a Diatomize::Descriptor.
//   Then we can write:
//
//      Diatom d = diatomize(x.getSD());    // Serialize x into a Diatom
//
//   Or:
//
//      antidiatomize(y.getSD(), d);        // Deserialize x from a Diatom
//
// -- BH 2015
// Published under the MIT license - http://opensource.org/licenses/MIT
//

#include <cstdio>
#include "../Diatomize.h"


// A custom serializer function
std::function<void(Diatom&, float&)> float_d_llambda = [](Diatom &d, float &x) {
  x = d.value__number;
};


class Y {
public:
  float areaOfEnclosure = 1.47;

  Diatomize::Descriptor getSD() {
    return {{ diatomPart("areaOfEnclosure", &areaOfEnclosure) }};
  }
};


// Serializable class – provides a SD
class X {
public:
  int monkeys    = 12;
  float penguins = 5.0;
  bool isOpen    = true;

  Y zooLayout;
  std::string zooName                   = "My Zoo";
  std::vector<float> penguinHeights     = { 1.0, 1.1, 1.2, 1.3 };
  std::vector<std::string> penguinNames = { "Jimmy", "Alice", "Eric", "Suzanne" };

  Diatomize::Descriptor getSD() {
    return {{
      diatomPart("monkeys", &monkeys),
      diatomPart("penguins", &penguins,
        [](float &x) -> Diatom  {  return Diatom((double) x);  },    // Inline custom serializer function
        float_d_llambda
      ),
      diatomPart("zooLayout", zooLayout.getSD(), &zooLayout),
      diatomPart("zooName", &zooName),
      diatomPart("isOpen", &isOpen),
      diatomPart("penguinHeights", &penguinHeights),
      diatomPart("penguinNames", &penguinNames)
    }};
  }

  void print() {
    printf(
      "%s: %5d, %5.2f, %5.2f  (%s)\n",
      zooName.c_str(), monkeys, penguins, zooLayout.areaOfEnclosure,
      isOpen ? "open" : "closed for refurbishment"
    );
    printf("  penguin heights:");
    for (auto i : penguinHeights)
      printf("  %f", i);
    printf("\n");

    printf("  penguin names:");
    for (auto i : penguinNames)
      printf("  %s", i.c_str());
    printf("\n");
  }
};


int main() {
  X x;
  x.print();

  Diatom d = diatomize(x.getSD());
  // d._print();

  d["monkeys"]  = Diatom((double) 11);
  d["penguins"] = Diatom((double) 4);
  d["zooName"]  = Diatom("My Second Zoo");
  d["isOpen"]   = Diatom(false);
  d["zooLayout"]["areaOfEnclosure"] = Diatom(0.63);
  d["penguinHeights"]["2"] = 74.13;
  d["penguinHeights"]["3"] = -127.4;
  d["penguinNames"]["2"] = "Titan";

  X y;
  antidiatomize(y.getSD(), d);
  y.print();
}

