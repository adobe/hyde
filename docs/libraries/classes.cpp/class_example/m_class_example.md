---
layout: method
title: class_example
hyde:
  owner: __INLINED__
  brief: __INLINED__
  tags:
    - method
  inline:
    brief: _multiple descriptions_
    owner: fosterbrereton
  defined_in_file: classes.cpp
  is_ctor: true
  overloads:
    class_example():
      annotation:
        - defaulted
      description: __INLINED__
      inline:
        description:
          - default constructor.
      signature_with_names: class_example()
    explicit class_example(int):
      arguments:
        - description: __OPTIONAL__
          name: x
          type: int
      description: __INLINED__
      inline:
        arguments:
          x:
            description: The one integer parameter this routine takes
        description:
          - an explicit constructor that takes a single `int`.
      signature_with_names: explicit class_example(int x)
---
