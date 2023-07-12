---
layout: method
title: overloaded
hyde:
  owner: __INLINED__
  brief: __INLINED__
  tags:
    - method
  inline:
    brief: _multiple descriptions_
    owner: fosterbrereton
  defined_in_file: classes.cpp
  overloads:
    void overloaded():
      annotation:
        - deprecated
      description: __INLINED__
      inline:
        description:
          - a deprecated overload that takes zero parameters.
      return: __OPTIONAL__
      signature_with_names: void overloaded()
    void overloaded(const std::string &):
      arguments:
        - description: __OPTIONAL__
          name: first
          type: const std::string &
      description: __INLINED__
      inline:
        arguments:
          first:
            description: the first parameter of the first overload.
        brief: A series of overloaded functions.
        description:
          - an overloaded member function that takes one parameter.
      return: __OPTIONAL__
      signature_with_names: void overloaded(const std::string & first)
    void overloaded(const std::string &, class_example *, int):
      arguments:
        - description: __OPTIONAL__
          name: unnamed-0
          type: const std::string &
          unnamed: true
        - description: __OPTIONAL__
          name: unnamed-1
          type: class_example *
          unnamed: true
        - description: __OPTIONAL__
          name: unnamed-2
          type: int
          unnamed: true
      description: __INLINED__
      inline:
        description:
          - an overloaded member function that takes three unnamed parameters. Let it be known that Doxygen doesn't support documenting unnamed parameters at this time. There is a [bug open on the issue](https://github.com/doxygen/doxygen/issues/6926), but as of this writing does not appear to be progressing.
      return: __OPTIONAL__
      signature_with_names: void overloaded(const std::string &, class_example *, int)
    void overloaded(const std::string &, class_example *, int, bool, std::size_t):
      arguments:
        - description: __OPTIONAL__
          name: first
          type: const std::string &
        - description: __OPTIONAL__
          name: second
          type: class_example *
        - description: __OPTIONAL__
          name: third
          type: int
        - description: __OPTIONAL__
          name: fourth
          type: bool
        - description: __OPTIONAL__
          name: fifth
          type: std::size_t
      description: __INLINED__
      inline:
        arguments:
          fifth:
            description: the fifth parameter of the fourth overload.
          first:
            description: the first parameter of the fourth overload.
          fourth:
            description: the fourth parameter of the fourth overload.
          second:
            description: the second parameter of the fourth overload.
          third:
            description: the third parameter of the fourth overload.
        description:
          - an overloaded member function that takes _five_ parameters.
      return: __OPTIONAL__
      signature_with_names: void overloaded(const std::string & first, class_example * second, int third, bool fourth, std::size_t fifth)
    void overloaded(const std::string &, const std::string &) volatile:
      arguments:
        - description: __OPTIONAL__
          name: first
          type: const std::string &
        - description: __OPTIONAL__
          name: second
          type: const std::string &
      description: __INLINED__
      inline:
        arguments:
          first:
            description: the first parameter of the second overload.
          second:
            description: the second parameter of the second overload.
        brief: Another brief describing one of the overloaded functions.
        description:
          - an overloaded member function that takes two parameters.
      return: __OPTIONAL__
      signature_with_names: void overloaded(const std::string & first, const std::string & second) volatile
    void overloaded(const std::string &, std::vector<int>) const:
      arguments:
        - description: __OPTIONAL__
          name: first
          type: const std::string &
        - description: __OPTIONAL__
          name: second
          type: std::vector<int>
      description: __INLINED__
      inline:
        arguments:
          first:
            description: the first parameter of the third overload.
          second:
            description: the second parameter of the third overload.
        description:
          - another overloaded member function that takes two parameters.
      return: __OPTIONAL__
      signature_with_names: void overloaded(const std::string & first, std::vector<int> second) const
---
