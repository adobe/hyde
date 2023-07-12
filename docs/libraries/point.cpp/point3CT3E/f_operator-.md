---
layout: function
title: operator-
hyde:
  owner: __MISSING__
  brief: __INLINED__
  tags:
    - function
  inline:
    brief:
      - Subtraction operator.
  defined_in_file: point.cpp
  overloads:
    constexpr point<T> operator-(const point<T> &, const point<T> &):
      arguments:
        - description: __OPTIONAL__
          name: a
          type: const point<T> &
        - description: __OPTIONAL__
          name: b
          type: const point<T> &
      description: __INLINED__
      inline:
        arguments:
          a:
            description: The point to be subtracted from.
          b:
            description: The point to subtract.
        description:
          - Subtraction operator.
        return: A new point whose axis values are subtractions of the two inputs' axis values.
      return: __OPTIONAL__
      signature_with_names: constexpr point<T> operator-(const point<T> & a, const point<T> & b)
---
