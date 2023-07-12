---
layout: class
title: nested_class_example
hyde:
  owner: __INLINED__
  brief: __INLINED__
  tags:
    - class
  inline:
    brief: a class definition contained within `example_class`.
    description:
      - Note that nested classes will not inherit their ownership from the class that contains them, so thus need their own `hyde-owner`.
    owner: sean-parent
  defined_in_file: classes.cpp
  declaration: "\nstruct class_example::nested_class_example;"
  ctor: unspecified
  dtor: unspecified
  fields:
    _x:
      description: __INLINED__
      inline:
        description:
          - member field `_x` within the nested class example.
      type: int
    _y:
      description: __INLINED__
      inline:
        description:
          - member field `_y` within the nested class example.
      type: int
---
