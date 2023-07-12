---
layout: class
title: class_example
hyde:
  owner: __INLINED__
  brief: __INLINED__
  tags:
    - class
  inline:
    brief: an example class that demonstrates what Hyde documents.
    owner: fosterbrereton
  defined_in_file: classes.cpp
  declaration: "\nclass class_example;"
  dtor: unspecified
  typedefs:
    typedef_example:
      definition: std::string
      description: __INLINED__
      inline:
        description:
          - a nested `typedef` expression.
    using_example:
      definition: std::string
      description: __INLINED__
      inline:
        description:
          - a nested `using` expression.
  fields:
    _deprecated_member:
      annotation:
        - private
        - deprecated("example deprecation message")
      description: __INLINED__
      inline:
        description:
          - a deprecated member variable that contains a message. Apparently this works?!
      type: int
    _nested:
      annotation:
        - private
      description: __INLINED__
      inline:
        description:
          - an instance of the nested class example defined earlier.
      type: class_example::nested_class_example
    _static_member:
      description: __INLINED__
      inline:
        description:
          - static member variable.
      type: const int
    _x:
      annotation:
        - private
      description: __INLINED__
      inline:
        description:
          - some variable that holds an integer.
      type: int
---
