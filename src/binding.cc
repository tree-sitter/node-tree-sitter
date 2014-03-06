#include <node.h>
#include <v8.h>
#include "tree_sitter/compiler.h"

using namespace v8;
using namespace tree_sitter::rules;
using tree_sitter::Grammar;
using std::map;
using std::string;
using std::pair;
using std::vector;

void throw_type_error(const char *message) {
  ThrowException(Exception::TypeError(String::New(message)));
}

template<typename T>
Handle<T> js_object_get(Handle<Object> object, const char *key) {
  return Handle<T>::Cast(object->Get(String::NewSymbol(key)));
}

template<typename T>
Handle<T> js_array_get(Handle<Array> array, uint32_t i) {
  return Handle<T>::Cast(array->Get(i));
}

string string_from_js_string(Handle<String> js_string) {
  int len = js_string->Utf8Length();
  char *c_string = new char[len + 1];
  js_string->WriteUtf8(c_string, len, NULL, 0);
  return string(c_string);
}

string name_from_js_grammar(Handle<Object> js_grammar) {
  Handle<String> name = js_object_get<String>(js_grammar, "name");
  if (!name->IsString()) throw_type_error("Expected grammar name to be a string");
  return string_from_js_string(name);
}

string start_rule_name_from_js_grammar(Handle<Object> js_grammar) {
  Handle<String> start = js_object_get<String>(js_grammar, "start");
  if (!start->IsString()) throw_type_error("Expected grammar start to be a string");
  return string_from_js_string(start);
}

rule_ptr rule_from_js_rule(Handle<Object> js_rule) {
  if (!js_rule->IsObject()) throw_type_error("Expected rule to be an object");
  Handle<String> js_type = js_object_get<String>(js_rule, "type");
  if (!js_type->IsString()) throw_type_error("Expected rule type to be a string");
  string type = string_from_js_string(js_type);

  if (type == "STRING") {
    return str(string_from_js_string(js_object_get<String>(js_rule, "value")));
  }
  if (type == "PAT") {
    return pattern(string_from_js_string(js_object_get<String>(js_rule, "value")));
  }
  if (type == "SYMBOL") {
    return sym(string_from_js_string(js_object_get<String>(js_rule, "name")));
  }
  if (type == "CHOICE") {
    Handle<Array> js_members = js_object_get<Array>(js_rule, "members");
    vector<rule_ptr> members;
    uint32_t length = js_members->Length();
    for (uint32_t i = 0; i < length; i++) {
      Handle<Object> js_member = js_array_get<Object>(js_members, i);
      members.push_back(rule_from_js_rule(js_member));
    }
    return choice(members);
  }
  if (type == "SEQ") {
    Handle<Array> js_members = js_object_get<Array>(js_rule, "members");
    vector<rule_ptr> members;
    uint32_t length = js_members->Length();
    for (uint32_t i = 0; i < length; i++) {
      Handle<Object> js_member = js_array_get<Object>(js_members, i);
      members.push_back(rule_from_js_rule(js_member));
    }
    return seq(members);
  }
  if (type == "BLANK") {
    return blank();
  }
  if (type == "REPEAT") {
    Handle<Object> content = js_object_get<Object>(js_rule, "value");
    return repeat(rule_from_js_rule(content));
  }
  if (type == "ERROR") {
    Handle<Object> content = js_object_get<Object>(js_rule, "value");
    return err(rule_from_js_rule(content));
  }

  ThrowException(Exception::TypeError(
    String::Concat(String::New("Unexpected rule type: "), js_type)));
  return blank();
}

map<const string, const rule_ptr> rules_from_js_grammar(Handle<Object> js_grammar) {
  Handle<Object> js_rules = js_object_get<Object>(js_grammar, "rules");
  if (!js_rules->IsObject()) throw_type_error("Expected grammar rules to be an object");

  map<const string, const rule_ptr> result;
  Local<Array> rule_names = js_rules->GetOwnPropertyNames();
  uint32_t length = rule_names->Length();
  for (uint32_t i = 0; i < length; i++) {
    Local<String> js_rule_name = Local<String>::Cast(rule_names->Get(i));
    string rule_name = string_from_js_string(js_rule_name);
    rule_ptr rule = rule_from_js_rule(Handle<Object>::Cast(js_rules->Get(js_rule_name)));
    result.insert(pair<const string, const rule_ptr>(rule_name, rule));
  }

  return result;
}

Grammar grammar_from_js_grammar(Handle<Object> js_grammar) {
  string start_rule_name = start_rule_name_from_js_grammar(js_grammar);
  map<const string, const rule_ptr> rules = rules_from_js_grammar(js_grammar);
  return Grammar(start_rule_name, rules);
}

Handle<Value> CompileFn(const Arguments& args) {
  HandleScope scope;

  Handle<Object> js_grammar = Handle<Object>::Cast(args[0]);
  if (!js_grammar->IsObject())
    throw_type_error("Expected grammar to be an object");

  string name = name_from_js_grammar(js_grammar);
  Grammar grammar = grammar_from_js_grammar(js_grammar);
  string code = tree_sitter::compile(grammar, name);

  return scope.Close(String::New(code.c_str()));
}

void Init(Handle<Object> exports) {
  exports->Set(
    String::NewSymbol("compile"),
    FunctionTemplate::New(CompileFn)->GetFunction());
}

NODE_MODULE(tree_sitter_binding, Init)
