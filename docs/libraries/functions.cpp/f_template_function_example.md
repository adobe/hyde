---
layout: function
title: template_function_example
hyde:
  owner: __MISSING__
  brief: __INLINED__
  tags:
    - function
  inline:
    brief: _multiple descriptions_
  defined_in_file: functions.cpp
  overloads:
    int template_function_example():
      description: __INLINED__
      inline:
        description:
          - an example specialization of the template function example
        return: Forty-two
      return: __OPTIONAL__
      signature_with_names: int template_function_example()
    "template <class T>\nT template_function_example()":
      description: __INLINED__
      inline:
        description:
          - an example template function, deleted by default
        return: Not applicable, seeing that the default definition has been deleted.
      return: __OPTIONAL__
      signature_with_names: "template <class T>\nT template_function_example()"
---
