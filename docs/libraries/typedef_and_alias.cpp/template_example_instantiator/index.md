---
layout: class
title: template_example_instantiator
hyde:
  owner: __MISSING__
  brief: __INLINED__
  tags:
    - class
  inline:
    brief: Example struct that leverages type aliases defined above.
  defined_in_file: typedef_and_alias.cpp
  declaration: "\nstruct template_example_instantiator;"
  ctor: unspecified
  dtor: unspecified
  typedefs:
    typedef_full_specialization_example:
      definition: using_partial_specialization_example<double>
      description: __INLINED__
      inline:
        description:
          - Example partial specialization using `typedef`
    using_full_specialization_example:
      definition: using_partial_specialization_example<bool>
      description: __INLINED__
      inline:
        description:
          - Example full specialization
---
