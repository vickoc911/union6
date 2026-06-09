#!/usr/bin/env python

# SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
# SPDX-FileCopyrightText: 2026 Arjen Hiemstra <ahiemstra@heimr.nl>

# import copy
import dataclasses
from pathlib import Path
# import shutil
from typing import Any # Optional, Type
# import re

import ruamel.yaml as yaml
import jinja2

import lark


base_directory = Path(__file__).parent
root_directory = base_directory.parent.parent
doc_directory = root_directory / "doc"
# qml_directory = root_directory / "src" / "output" / "qtquick" / "style"


def mapping_value_node(mapping_node: yaml.MappingNode, key: str) -> yaml.Node | None:
    for key_node, value_node in mapping_node.value:
        if key_node.value == key:
            return value_node

    return None


def mapping_value(mapping_node: yaml.MappingNode, key: str, default_value: Any = None) -> Any:
    node = mapping_value_node(mapping_node, key)
    if node is not None:
        return node.value
    else:
        return default_value


@dataclasses.dataclass
class Hint:
    name: str
    description: str


@dataclasses.dataclass
class Attribute:
    name: str
    description: str = ""
    values: dict[str, str] = dataclasses.field(default_factory=dict)


@dataclasses.dataclass
class Element:
    name: str = ""
    group: str = ""
    description: str = ""

    type: str = ""
    states: list[str] = dataclasses.field(default_factory=list)
    hints: list[Hint] = dataclasses.field(default_factory=list)
    attributes: list[Attribute] = dataclasses.field(default_factory=list)
    subelements: list["Element"] = dataclasses.field(default_factory=list)

    def has_details(self):
        return self.type or self.states or self.hints or self.attributes


# Minimal grammar to parse QML files
# parser = lark.Lark(r"""
#     ?start: document
#
#     document: (pragma | import_statement | item)*
#
#     pragma: "pragma" pragma_name (":" pragma_value)? ";"?
#
#     pragma_name: identifier
#
#     pragma_value: identifier | STRING | NUMBER
#
#     import_statement: "import" import_path ("as" import_alias)?
#
#     import_path: qualified_identifier | STRING
#
#     import_alias: identifier
#
#     item: item_type "{" property_list "}"
#
#     item_type: qualified_identifier
#
#     ?qualified_identifier.2: identifier ("." identifier)*
#
#     property_list: property*
#
#     property: grouped_property
#             | simple_property
#             | item
#
#     grouped_property: property_name "{" property_list "}"
#
#     simple_property: property_name ":" expression (";" property_name ":" expression)*
#
#     property_name: qualified_identifier
#
#     expression: qualified_identifier
#               | literal
#               | item
#               | array
#               | block
#               # | js
#
#     array: "[" array_element ("," array_element)* ","? "]"
#
#     array_element: item
#                  | qualified_identifier
#
#     block: "{" expression* "}"
#
#     # js: /[^}\]]+/
#
#     ?literal: STRING
#             | NUMBER
#             | BOOL
#             | COLOR
#
#     ?identifier: /[a-zA-Z_][a-zA-Z0-9_]*/
#
#     COLOR: /#[0-9a-fA-F]{6}([0-9a-fA-F]{2})?/
#
#     STRING: ESCAPED_STRING
#
#     NUMBER: SIGNED_NUMBER
#
#     BOOL: "true" | "false"
#
#     SINGLE_LINE_COMMENT: /\/\/[^\n]*/
#
#     MULTI_LINE_COMMENT: /\/\*[\s\S]*?\*\//
#
#     %import common.ESCAPED_STRING
#     %import common.SIGNED_NUMBER
#     %import common.WS
#
#     %ignore WS
#     %ignore SINGLE_LINE_COMMENT
#     %ignore MULTI_LINE_COMMENT
# """, start = "start", ambiguity = "forest") #, lexer="dynamic_complete", ambiguity="explicit")


def process_node(node):
    if not isinstance(node, yaml.MappingNode):
        raise RuntimeError(f"Node {node} is not a mapping node")

    element = Element()

    for key_node, value_node in node.value:
        if key_node.value == "name":
            element.name = value_node.value

        if key_node.value == "group":
            element.group = value_node.value

        if key_node.value == "description":
            element.description = value_node.value

        if key_node.value == "type":
            element.type = value_node.value

        if key_node.value == "states":
            for node in value_node.value:
                element.states.append(node.value)

        if key_node.value == "hints":
            for hint_name, hint_description in value_node.value:
                element.hints.append(Hint(hint_name.value, hint_description.value))

        if key_node.value == "attributes":
            for attribute_name, attribute_description in value_node.value:
                attribute = Attribute(attribute_name.value)
                attribute.description = mapping_value(attribute_description, "description", "")

                values = mapping_value(attribute_description, "values", [])
                for key, value in values:
                    attribute.values[key.value] = value.value

                element.attributes.append(attribute)

        if key_node.value == "subelements":
            for node in value_node.value:
                element.subelements.append(process_node(node))

    return element

            # element.states = value_node.value
# @dataclasses.dataclass
# class Description:
#     name: str
#     type: str
#     parent: Optional["Description"] = None
#     children: list["Description"] = dataclasses.field(default_factory=list)
#
#     system_includes: dict[str, set[str]] = dataclasses.field(default_factory=dict)
#     local_includes: dict[str, set[str]] = dataclasses.field(default_factory=dict)
#     extra_code: dict[str, str] = dataclasses.field(default_factory=dict)
#
#     api_documentation: str = ""
#     css_documentation: str = ""
#
#     def __lt__(self, other):
#         return self.type < other.type
#
#     def add_system_include(self, file: str, include: str) -> None:
#         if not file in self.system_includes:
#             self.system_includes[file] = set()
#         self.system_includes[file].add(include)
#
#     def add_local_include(self, file: str, include: str) -> None:
#         if not file in self.local_includes:
#             self.local_includes[file] = set()
#         self.local_includes[file].add(include)
#
#     def add_child(self, child: "Description") -> None:
#         self.children.append(child)
#         if child.children:
#             self.add_local_include("property.h.j2", child.type + ".h")
#
#
# def ucfirst(value):
#     return f"{value[0].upper()}{value[1:]}"
#
#
# def group_name(type_name):
#     return f"{ucfirst(type_name)}PropertyGroup"
#
#
# def css_name(name):
#     parts = re.split(r"([A-Z][a-z]+)", name)
#     result = "-".join(part.lower() for part in parts if part)
#     return result.replace("--", "-")
#
#
# def process_node(node, name: str, parent: Description, memo: dict[str, Description], type_name: str | None = None):
#     if not isinstance(node, yaml.MappingNode):
#         raise RuntimeError(f"Node {node} is not a mapping node!")
#
#     node_type = mapping_value(node, "type")
#     if node_type is None:
#         raise RuntimeError(f"Node {node} is missing a type!")
#
#     if not type_name:
#         type_name = node_type
#
#     description_type = node_type
#     if node_type == "group" or node_type == "type":
#         description_type = group_name(type_name)
#
#     description = None
#     if description_type in memo:
#         description = copy.deepcopy(memo[description_type])
#         description.name = name
#         description.parent = parent
#     else:
#         description = Description(name, description_type, parent)
#         if node_type == "group" or node_type == "type":
#             memo[type_name] = description
#
#     for key_node, value_node in node.value:
#         if key_node.value == "extra_code":
#             for template_name, extra_code in value_node.value:
#                 if isinstance(extra_code, yaml.MappingNode):
#                     extra_code_data = {}
#                     for identifier, code in extra_code.value:
#                         extra_code_data[identifier.value] = code.value
#                     description.extra_code[template_name.value] = extra_code_data
#                 else:
#                     description.extra_code[template_name.value] = extra_code.value
#
#         elif key_node.value == "extra_system_includes":
#             for template_name, includes in value_node.value:
#                 for include_name in includes.value:
#                     description.add_system_include(template_name.value, include_name.value)
#
#         elif key_node.value == "doc":
#             if isinstance(value_node, yaml.MappingNode):
#                 description.api_documentation = mapping_value(value_node, "api", "")
#                 description.css_documentation = mapping_value(value_node, "css", "")
#             else:
#                 description.api_documentation = value_node.value
#                 description.css_documentation = value_node.value
#
#         elif key_node.value == "types":
#             for key_node, value_node in value_node.value:
#                 memo = memo | process_node(value_node, key_node.value, description, memo, type_name = key_node.value)
#
#         elif key_node.value == "children":
#             for key_node, value_node in value_node.value:
#                 prop = None
#
#                 if isinstance(value_node, AliasNode):
#                     child = copy.deepcopy(memo[value_node.value])
#                     child.name = key_node.value
#                     child.parent = description
#                     description.add_child(child)
#                 else:
#                     child_type_name = value_node.anchor if value_node.anchor is not None else key_node.value
#                     memo = memo | process_node(value_node, key_node.value, description, memo, type_name = child_type_name)
#
#     for entry in include_patterns:
#         if not description.type.startswith(entry["pattern"]) or not parent:
#             continue
#
#         system = entry.get("system_include", False)
#         include = entry.get("use_include", description.type if system else description.type + ".h")
#
#         if include is None:
#             continue
#
#         if system:
#             parent.add_system_include("property.h.j2", include)
#         else:
#             parent.add_local_include("property.h.j2", include)
#
#     if parent and node_type != "type":
#         parent.add_child(description)
#
#     return memo
#
#
# @jinja2.pass_context
# def render_template_filter(context, value, **kwargs):
#     return context.environment.from_string(value).render(context, **kwargs)


def render_template(template_name: str, output_path: Path, env: jinja2.Environment, data: dict):
    render_data = data.copy()
    render_data["extra_code"] = data.get("extra_code", {}).get(template_name, "")
    render_data["system_includes"] = data.get("system_includes", {}).get(template_name, [])
    render_data["local_includes"] = data.get("local_includes", {}).get(template_name, [])
    render_data["api_documentation"] = data.get("api_documentation", "")
    render_data["css_documentation"] = data.get("css_documentation", "")

    with open(output_path, "w") as f:
        template = jinja_env.get_template(template_name, None)
        f.write(template.render(render_data))


# class DocTransformer(lark.Transformer):
#     identifier = str
#
#     def qualified_identifier(self, args):
#         return ".".join(args)
#
#     def SINGLE_LINE_COMMENT(self, args):
#         comment = str(args)
#         if comment.startswith("//!"):
#             return comment[3:].strip()
#         else:
#             return lark.Discard
#
#     def MULTI_LINE_COMMENT(self, args):
#         comment = str(args)
#         if comment.startswith("/*!"):
#             result = ""
#             for line in comment.replace("/*!", "").replace("*/", "").strip().split("\n"):
#                 result += line.lstrip("* ") + "\n"
#             return result.strip()
#         else:
#             return lark.Discard


if __name__ == "__main__":
    structure = None

    parser = yaml.YAML()
    # parser.Composer = PreserveAliasesComposer

    types = []

    for path in base_directory.glob("*.yml"):
        with open(path) as f:
            structure = parser.compose(f)

            for node in structure.value:
                types.append(process_node(node))

    jinja_env = jinja2.Environment(
        loader=jinja2.FileSystemLoader(base_directory),
        autoescape=False,
        trim_blocks=True,
        lstrip_blocks=True,
    )

    output_path = doc_directory / "elements.qdoc"
    if output_path.exists():
        output_path.unlink()

    render_template("elements.qdoc.j2", output_path, jinja_env, {"types": types})

    # jinja_env.filters["ucfirst"] = ucfirst
    # jinja_env.filters["render"] = render_template_filter
    # jinja_env.filters["css_name"] = css_name
    #
    # shutil.rmtree(src_directory, ignore_errors = True)
    # shutil.rmtree(tests_directory, ignore_errors = True)
    # shutil.rmtree(quick_output_directory, ignore_errors = True)
    #
    # css_generated_path = css_input_directory / "generated-properties.css"
    # if css_generated_path.exists():
    #     css_generated_path.unlink()
    #
    # css_doc_generated_path = css_docs_directory / "css-properties.qdoc"
    # if css_doc_generated_path.exists():
    #     css_doc_generated_path.unlink()
    #
    # src_directory.mkdir(exist_ok = True)
    # tests_directory.mkdir(exist_ok = True)
    # css_input_directory.mkdir(exist_ok = True)
    # quick_output_directory.mkdir(exist_ok = True)
    #
    # for name, type_definition in types.items():
    #     data = {field.name: getattr(type_definition, field.name) for field in dataclasses.fields(type_definition)}
    #
    #     type_name = type_definition.type
    #
    #     render_template("property.h.j2", (src_directory / type_name).with_suffix(".h"), jinja_env, data)
    #     render_template("property.cpp.j2", (src_directory / type_name).with_suffix(".cpp"), jinja_env, data)
    #
    #     render_template("autotest.cpp.j2", (tests_directory / ("Test" + type_name)).with_suffix(".cpp"), jinja_env, data)
    #
    #     render_template("qml_group.h.j2", (quick_output_directory / (type_name + "Quick")).with_suffix(".h"), jinja_env, data)
    #     render_template("qml_group.cpp.j2", (quick_output_directory / (type_name + "Quick")).with_suffix(".cpp"), jinja_env, data)
    #
    # data = {"types": types.values()}
    #
    # render_template("CreateTestInstances.h.j2", tests_directory / "CreateTestInstances.h", jinja_env, data)
    # render_template("CMakeLists.txt.j2", src_directory / "CMakeLists.txt", jinja_env, {"target_name": "Union", "file_suffix": ""} | data)
    # render_template("CMakeLists.tests.txt.j2", tests_directory / "CMakeLists.txt", jinja_env, data)
    # render_template("CMakeLists.txt.j2", quick_output_directory / "CMakeLists.txt", jinja_env, {"target_name": "UnionQuickImpl", "file_suffix": "Quick"} | data)
    #
    # render_template("properties.css.j2", css_generated_path, jinja_env, data)
    # render_template("css-properties.qdoc.j2", css_doc_generated_path, jinja_env, data)
