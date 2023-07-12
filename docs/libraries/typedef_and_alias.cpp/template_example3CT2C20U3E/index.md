---
layout: class
title: template_example<T, U>
hyde:
  owner: __MISSING__
  brief: __INLINED__
  tags:
    - class
  inline:
    brief: Example class with two type definitions
  defined_in_file: typedef_and_alias.cpp
  declaration: "template <typename T, typename U>\nstruct template_example;"
  ctor: unspecified
  dtor: unspecified
  typedefs:
    typedef_from_T:
      definition: T
      description: __INLINED__
      inline:
        description:
          - Type derived from the first template parameter.
    using_from_U:
      definition: U
      description: __INLINED__
      inline:
        description:
          - Type derived from the second template parameter.
---
