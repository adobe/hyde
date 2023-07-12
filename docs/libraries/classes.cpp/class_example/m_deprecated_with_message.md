---
layout: method
title: deprecated_with_message
hyde:
  owner: __INLINED__
  brief: __INLINED__
  tags:
    - method
  inline:
    brief:
      - deprecated member function that contains a compile-time deprecation message.
    owner: fosterbrereton
  defined_in_file: classes.cpp
  overloads:
    void deprecated_with_message(const std::string &, class_example *):
      annotation:
        - deprecated("example deprecation message")
      arguments:
        - description: __OPTIONAL__
          name: s
          type: const std::string &
        - description: __OPTIONAL__
          name: f
          type: class_example *
      description: __INLINED__
      inline:
        arguments:
          f:
            description: the second parameter
          s:
            description: the first parameter
        description:
          - deprecated member function that contains a compile-time deprecation message.
      return: __OPTIONAL__
      signature_with_names: void deprecated_with_message(const std::string & s, class_example * f)
---
