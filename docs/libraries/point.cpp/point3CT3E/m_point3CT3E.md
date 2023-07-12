---
layout: method
title: point<T>
hyde:
  owner: __MISSING__
  brief: __INLINED__
  tags:
    - method
  inline:
    brief: _multiple descriptions_
  defined_in_file: point.cpp
  is_ctor: true
  overloads:
    point<T>():
      annotation:
        - defaulted
      description: __INLINED__
      inline:
        description:
          - Default constructor of default definition.
      signature_with_names: point<T>()
    point<T>(T, T):
      arguments:
        - description: __OPTIONAL__
          name: _x
          type: T
        - description: __OPTIONAL__
          name: _y
          type: T
      description: __INLINED__
      inline:
        arguments:
          _x:
            description: The `x` coordinate to sink.
          _y:
            description: The `y` coordinate to sink.
        description:
          - Value-based constructor that takes `x` and `y` values and sinks them into place.
      signature_with_names: point<T>(T _x, T _y)
---
