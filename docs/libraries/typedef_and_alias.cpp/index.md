---
layout: library
title: typedef_and_alias.cpp
hyde:
  owner: __MISSING__
  brief: __MISSING__
  tags:
    - sourcefile
  library-type: sourcefile
  typedefs:
    typedef_example:
      definition: int
      description: __INLINED__
      inline:
        description:
          - Example typedef expression whose underlying type is `int`.
    typedef_full_specialization_example:
      definition: using_partial_specialization_example<double>
      description: __INLINED__
      inline:
        description:
          - Using typedef to define another full specialization of the above partial specialization
    using_example:
      definition: int
      description: __INLINED__
      inline:
        description:
          - Example using expression whose underlying type is `int`.
    using_full_specialization_example:
      definition: using_partial_specialization_example<bool>
      description: __INLINED__
      inline:
        description:
          - Full specialization of the above partial specialization
    using_partial_specialization_example:
      definition: template_example<bool, U>
      description: __INLINED__
      inline:
        description:
          - Partial specialization of the above `template_example` template
---
