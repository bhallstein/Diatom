//
// Diatomize.h - BH, Apr. 2015
//

#ifndef __Diatomize_h
#define __Diatomize_h

#include <vector>
#include "../Diatom.h"

namespace Diatomize {

  #pragma mark - Serializing helper functions

  inline Diatom _serialize(int &x)    {  return Diatom((double) x);  }
  inline Diatom _serialize(float  &x) {  return Diatom(x);  }
  inline Diatom _serialize(double &x) {  return Diatom(x);  }
  inline Diatom _serialize(bool &x)   {  return Diatom(x);  }
  inline Diatom _serialize(std::string &x) {  return Diatom(x);  }
  inline Diatom _serialize(std::string *x) {  return Diatom(*x); }
  template <typename T>
  inline Diatom _serialize(std::vector<T> &vec) {
    Diatom d;
    for (int i=0, n=vec.size(); i < n; ++i)
      d[std::to_string(i)] = Diatom(vec[i]);
    return d;
  }

  inline void _deserialize(Diatom &d, int &x)    {  x = d.value__number;  }
  inline void _deserialize(Diatom &d, float &x)  {  x = d.value__number;  }
  inline void _deserialize(Diatom &d, double &x) {  x = d.value__number;  }
  inline void _deserialize(Diatom &d, bool &b)   {  b = d.value__bool; }
  inline void _deserialize(Diatom &d, std::string &x) {  x = d.value__string;  }
  inline void _deserialize(Diatom &d, std::string *x) {  x = new std::string(d.value__string); }
  template <typename T>
  inline void _deserialize(Diatom &d, std::vector<T> &vec) {
    vec.clear();
    d.each([&](std::string &s, Diatom &d) {
      T x;
      _deserialize(d, x);
      vec.push_back(x);
    });
  }


  #pragma mark - SerializerBase

  class SerializerBase {
    std::string name;
  public:
    SerializerBase(const std::string &_name) : name(_name) {  }
    virtual ~SerializerBase() { }
    std::string getName() { return name; }
    virtual Diatom convertToDiatom() = 0;
    virtual void   convertFromDiatom(Diatom &) = 0;
    virtual SerializerBase* clone() = 0;
  };

  template<class Derived>
  class SerializerCRTP : public SerializerBase {
  public:
    SerializerCRTP(const std::string &_name) : SerializerBase(_name) { }
    SerializerBase* clone() {
      return new Derived((const Derived &)*this);
    }
  };


  #pragma mark - Serializer

  template <typename ptype>
  class Serializer : public SerializerCRTP<Serializer<ptype>> {
    ptype *p;

  public:
    Serializer(const std::string &_name, ptype *_p) :
      SerializerCRTP<Serializer<ptype>>(_name),
      p(_p)
    {  }

    Diatom convertToDiatom() {
      return _serialize(*p);
    }
    void convertFromDiatom(Diatom &d) {
      _deserialize(d, *p);
    }
  };


  #pragma mark - CustomSerializer

  template <typename ptype, typename s_func, typename d_func>
  class CustomSerializer : public SerializerCRTP<CustomSerializer<ptype, s_func, d_func>> {
    ptype *p;

    s_func s_fn;
    d_func d_fn;

  public:
    CustomSerializer(const std::string &_name, ptype *_p, s_func _s, d_func _d) :
      SerializerCRTP<CustomSerializer<ptype, s_func, d_func>>(_name),
      p(_p),
      s_fn(_s),
      d_fn(_d)
    {  }

    Diatom convertToDiatom() {
      return s_fn(*p);
    }
    void convertFromDiatom(Diatom &d) {
      d_fn(d, *p);
    }
  };

  struct Descriptor {
    std::vector<SerializerBase*> descriptor;
    Descriptor() { }
    Descriptor(const std::vector<SerializerBase*> &_desc) : descriptor(_desc) { }
    Descriptor(const Descriptor &d) {
      for (auto i : d.descriptor)
        descriptor.push_back(i->clone());
    }
    Descriptor& operator=(const Descriptor &d) {
      if (this == &d) return *this;
      for (auto i : descriptor) delete i;
      for (auto i : d.descriptor) descriptor.push_back(i->clone());
      return *this;
    }
    ~Descriptor() { for (auto i : descriptor) delete i; }
  };
}

#pragma mark - diatomize() and antidiatomize()

Diatom diatomize(const Diatomize::Descriptor &sd);

void antidiatomize(const Diatomize::Descriptor &sd, Diatom &d);

namespace Diatomize {

  #pragma mark - CompoundSerializer

  template<typename ptype>
  class CompoundSerializer : public SerializerCRTP<CompoundSerializer<ptype>> {
    Descriptor sd;
    // ptype *p;

  public:
    CompoundSerializer(const std::string &_name, const Descriptor &_sd, ptype *_p) :
      SerializerCRTP<CompoundSerializer<ptype>>(_name),
      sd(_sd)
      // p(_p)
    {

    }

    Diatom convertToDiatom() {
      return diatomize(sd);
    }
    void convertFromDiatom(Diatom &d) {
      antidiatomize(sd, d);
    }

  };
}

#pragma mark - MakeSerializer functions

template<typename ptype>
Diatomize::SerializerBase*
diatomPart(const std::string &name, ptype *p) {
  return new Diatomize::Serializer<ptype>(name, p);
}

template<typename ptype, class s_func, class d_func>
Diatomize::SerializerBase*
diatomPart(const std::string &name, ptype *p, s_func s, d_func d) {
  return new Diatomize::CustomSerializer<ptype, s_func, d_func>(name, p, s, d);
}

template<typename ptype>
Diatomize::SerializerBase*
diatomPart(const std::string &name, const Diatomize::Descriptor &sd, ptype *p) {
  return new Diatomize::CompoundSerializer<ptype>(name, sd, p);
}


#endif

