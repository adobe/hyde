---
layout: null
---
/*
Copyright 2020 Adobe. All rights reserved.
This file is licensed to you under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License. You may obtain a copy
of the License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under
the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR REPRESENTATIONS
OF ANY KIND, either express or implied. See the License for the specific language
governing permissions and limitations under the License.
*/

{% assign siblings = site.pages | group_by_exp: "e", "e.url | split: '/' | pop | join: '/' | append: '/'" | sort: "name" %}

window.hyde_index = {
    {% for sibling in siblings %}
        {% assign layouts = sibling.items | group_by: "layout" | sort: "name" %}
        {{ sibling.name | jsonify }}: {
            {% for layout in layouts %}
                {% case layout.name %}
                    {% when "directory" %}
                        {% assign directories = layout.items | sort: "title" %}
                        "directory": `<h3>Directories</h3>
                        <table class='associated-table'>
                                        {% for p in directories %}
                            <tr>
                                <td class='name'>
                                  <div><a href="{{p.url | relative_url}}"><code>{{ p.title | escape }}</code></a></div>
                                </td>
                                <td class='brief'>
                                    {%- if p.brief -%}
                                        {{ p.brief | markdownify }}
                                    {%- elsif p.description -%}
                                        {{ p.description | markdownify }}
                                    {%- else -%}
                                        {{ '_No details given_' | markdownify }}
                                    {%- endif -%}
                                    {%- if p.annotation -%}
                                        <span class='annotation'>({{p.annotation | join: ", "}})</span>
                                    {%- endif -%}
                                </td>
                            </tr>
                                        {% endfor %}
                        </table>`,
                    {% when "library" %}
                        {% assign sub_libs = layout.items | group_by: "library-type" | sort: "name" %}
                        {% for sub_lib in sub_libs %}
                            {% case sub_lib.name %}
                                {% when "sourcefile" %}
                                    {% assign sourcefiles = sub_lib.items | sort: "title" %}
                                    "sourcefile": `<h3>Sources</h3>
                                    <table class='associated-table'>
                                      {% for p in sourcefiles %}
                                        <tr>
                                            <td class='name'>
                                              <div><a href="{{p.url | relative_url}}"><code>{{ p.title | escape }}</code></a></div>
                                            </td>
                                            <td class='brief'>
                                                {% if p.brief %}
                                                    {{ p.brief | markdownify }}
                                                {% elsif p.description %}
                                                    {{ p.description | markdownify }}
                                                {% else %}
                                                    {{ '_No details given_' | markdownify }}
                                                {% endif %}
                                                {% if p.annotation %}
                                                    <span class='annotation'>({{p.annotation | join: ", "}})</span>
                                                {% endif %}
                                            </td>
                                        </tr>
                                      {% endfor %}
                                    </table>`,
                                {% when "library" %}
                                    {% assign libraries = sub_lib.items | sort: "title" %}
                                    "library": `<h3>Subcomponents</h3>
                                    <table class='associated-table'>
                                      {% for p in libraries %}
                                        <tr>
                                            <td class='name'>
                                              <div><a href="{{p.url | relative_url}}">{{ p.title | escape }}</a></div>
                                            </td>
                                            <td class='brief'>
                                                {% if p.brief %}
                                                    {{ p.brief | markdownify }}
                                                {% elsif p.description %}
                                                    {{ p.description | markdownify }}
                                                {% else %}
                                                    {{ '_No details given_' | markdownify }}
                                                {% endif %}
                                                {% if p.annotation %}
                                                    <span class='annotation'>({{p.annotation | join: ", "}})</span>
                                                {% endif %}
                                            </td>
                                        </tr>
                                      {% endfor %}
                                    </table>`,
                            {% endcase %}
                        {% endfor %}
                    {% when "class" %}
                        {% assign classes = layout.items | sort: "title" %}
                        "class": `<h3>Classes</h3>
                          <table class='associated-table'>
                            {% for p in classes %}
                              <tr>
                                <td class='name'>
                                  <div><a href="{{p.url | relative_url}}">{{ p.title | escape }}</a></div>
                                </td>
                                <td class='brief'>
                                  {% if p.brief %}
                                    {{ p.brief | markdownify }}
                                  {% elsif p.description %}
                                    {{ p.description | markdownify }}
                                  {% else %}
                                    {{ '_No details given_' | markdownify }}
                                  {% endif %}
                                  {% if p.annotation %}
                                    <span class='annotation'>({{p.annotation | join: ", "}})</span>
                                  {% endif %}
                                </td>
                              </tr>
                            {% endfor %}
                          </table>`,
                    {% when "function" %}
                        {% assign functions = layout.items | sort: "title" %}
                        "function": `<h3>Functions</h3>
                        <table class='associated-table'>
                          {% for p in functions %}
                            <tr>
                                <td class='name'>
                                  <div><a href="{{p.url | relative_url}}">{{ p.title | escape }}</a></div>
                                </td>
                                <td class='brief'>
                                    {% if p.brief %}
                                        {{ p.brief | markdownify }}
                                    {% elsif p.description %}
                                        {{ p.description | markdownify }}
                                    {% else %}
                                        {{ '_No details given_' | markdownify }}
                                    {% endif %}
                                    {% if p.annotation %}
                                        <span class='annotation'>({{p.annotation | join: ", "}})</span>
                                    {% endif %}
                                </td>
                            </tr>
                          {% endfor %}
                        </table>`,
                    {% when "method" %}
                        {% assign methods = layout.items %}
                        "method": `<h3>Member Functions</h3>
                        <table class='definition-table'>

                        {% for p in methods %}
                          {% if p.is_ctor %}
                            <tr>
                                <td class='decl' colspan='2'><a href='{{p.url | relative_url}}'>(constructor)</a></td>
                            </tr>
                          {% endif %}
                        {% endfor %}

                        {% for p in methods %}
                          {% if p.is_dtor %}
                            <tr>
                                <td class='decl' colspan='2'><a href='{{p.url | relative_url}}'>(destructor)</a></td>
                            </tr>
                          {% endif %}
                        {% endfor %}

                        {% for p in methods %}
                          {% if p.is_ctor or p.is_dtor %}
                            {% continue %}
                          {% endif %}

                          <tr>
                            <td class='decl'>
                              <div><a href="{{p.url | relative_url}}">{{ p.title | escape }}</a></div>
                            </td>
                            <td class='defn'>
                              {% if p.brief %}
                                {{ p.brief | markdownify}}
                              {% elsif p.description %}
                                {{ p.description | markdownify}}
                              {% else %}
                                {{ '_No details given_' | markdownify}}
                              {% endif %}
                              {% if p.annotation %}
                                <span class='annotation'>({{p.annotation | join: ", "}})</span>
                              {% endif %}
                            </td>
                          </tr>
                        {% endfor %}
                        </table>`,
                    {% when "enumeration" %}
                    {% assign enums = layout.items %}
                    "enumeration": `<h3>Enumerations</h3>
                    <table class='associated-table'>
                    {% for p in enums %}
                    <tr>
                      <td class='name'>
                      <div><a href="{{p.url | relative_url}}">{{ p.title | escape }}</a></div>
                      </td>
                      <td class='brief'>
                        {% if p.brief %}
                          {{ p.brief | markdownify}}
                        {% elsif p.description %}
                          {{ p.description | markdownify}}
                        {% else %}
                          {{ '_No details given_' | markdownify}}
                        {% endif %}
                        {% if p.annotation %}
                          <span class='annotation'>({{p.annotation | join: ", "}})</span>
                        {% endif %}
                      </td>
                    </tr>
                    {% endfor %}
                    </table>`,
                    {% when "page" %}
                    {% assign pages = layout.items | sort: "title" %}
                    "page": `{% for p in pages %}
                    <tr>
                        <td>
                            <i class="fa fa-book"></i>
                        </td>
                        <td>
                            <a href="{{ BASE_PATH }}{{ p.url }}">{{ p.title | markdownify }}</a>
                        </td>
                    </tr>
                    {% endfor %}`
                    {% when "eng_index" %}
                        {% assign subdocs = layout.items | sort: "title" %}
                        "eng_index":`{% for p in subdocs %}
                            <tr>
                                <td>
                                    <i class="fa fa-folder"></i>
                                </td>
                                <td>
                                    <a href="{{ BASE_PATH }}{{ p.url }}">{{ p.title | markdownify }}</a>
                                </td>
                            </tr>
                            {% endfor %}`

                {% endcase %}
            {% endfor %}
        },
    {%endfor%}
};

window.hyde_tabs = `
    {% assign tabbed = site.pages | where_exp:"p","p.tab" | sort:"tab"%}
    {% for p in tabbed %}
        <a class="page-link" href="{{ p.url | prepend: site.baseurl }}">{{ p.tab }}</a>
    {% endfor %}`;

window.hyde_title_index = {
    {% for p in site.pages %}
        {{ p.url | jsonify }}: {{ p.title | jsonify }},
    {% endfor %}
};
