---
layout: method
title: nested_class_example
hyde:
  owner: __INLINED__
  brief: __OPTIONAL__
  tags:
    - method
  inline:
    owner: sean-parent
  defined_in_file: classes.cpp
  is_ctor: true
  overloads:
    nested_class_example():
      annotation:
        - implicit
      description: __OPTIONAL__
      signature_with_names: nested_class_example()
    nested_class_example(class_example::nested_class_example &&):
      annotation:
        - implicit
      arguments:
        - description: __OPTIONAL__
          name: unnamed-0
          type: class_example::nested_class_example &&
          unnamed: true
      description: __OPTIONAL__
      signature_with_names: nested_class_example(class_example::nested_class_example &&)
    nested_class_example(const class_example::nested_class_example &):
      annotation:
        - implicit
      arguments:
        - description: __OPTIONAL__
          name: unnamed-0
          type: const class_example::nested_class_example &
          unnamed: true
      description: __OPTIONAL__
      signature_with_names: nested_class_example(const class_example::nested_class_example &)
---
