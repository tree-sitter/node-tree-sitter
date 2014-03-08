#include "./binding.h"
#include "tree_sitter/compiler.h"

using namespace v8;
using namespace tree_sitter::rules;
using tree_sitter::Grammar;
using std::map;
using std::string;
using std::pair;
using std::vector;

static inline std::string StringFromJsString(Handle<String> js_string) {
  String::Utf8Value utf8_string(js_string);
  return std::string(*utf8_string);
}

template<typename T>
Handle<T> ObjectGet(Handle<Object> object, const char *key) {
  return Handle<T>::Cast(object->Get(String::NewSymbol(key)));
}

template<typename T>
Handle<T> ArrayGet(Handle<Array> array, uint32_t i) {
  return Handle<T>::Cast(array->Get(i));
}

rule_ptr RuleFromJsRule(Handle<Object> js_rule) {
  if (!js_rule->IsObject())
    ThrowException(Exception::TypeError(String::New("Expected rule to be an object")));
  Handle<String> js_type = ObjectGet<String>(js_rule, "type");
  if (!js_type->IsString())
    ThrowException(Exception::TypeError(String::New("Expected rule type to be a string")));
  string type = StringFromJsString(js_type);

  if (type == "STRING") {
    return str(StringFromJsString(ObjectGet<String>(js_rule, "value")));
  }
  if (type == "PATTERN") {
    return pattern(StringFromJsString(ObjectGet<String>(js_rule, "value")));
  }
  if (type == "SYMBOL") {
    return sym(StringFromJsString(ObjectGet<String>(js_rule, "name")));
  }
  if (type == "CHOICE") {
    Handle<Array> js_members = ObjectGet<Array>(js_rule, "members");
    vector<rule_ptr> members;
    uint32_t length = js_members->Length();
    for (uint32_t i = 0; i < length; i++) {
      Handle<Object> js_member = ArrayGet<Object>(js_members, i);
      members.push_back(RuleFromJsRule(js_member));
    }
    return choice(members);
  }
  if (type == "SEQ") {
    Handle<Array> js_members = ObjectGet<Array>(js_rule, "members");
    vector<rule_ptr> members;
    uint32_t length = js_members->Length();
    for (uint32_t i = 0; i < length; i++) {
      Handle<Object> js_member = ArrayGet<Object>(js_members, i);
      members.push_back(RuleFromJsRule(js_member));
    }
    return seq(members);
  }
  if (type == "BLANK") {
    return blank();
  }
  if (type == "REPEAT") {
    Handle<Object> content = ObjectGet<Object>(js_rule, "value");
    return repeat(RuleFromJsRule(content));
  }
  if (type == "ERROR") {
    Handle<Object> content = ObjectGet<Object>(js_rule, "value");
    return err(RuleFromJsRule(content));
  }

  ThrowException(Exception::TypeError(String::Concat(String::New("Unexpected rule type: "), js_type)));
  return blank();
}

Grammar GrammarFromJsGrammar(Handle<Object> js_grammar) {
  Handle<String> start = ObjectGet<String>(js_grammar, "start");
  if (!start->IsString())
    ThrowException(Exception::TypeError(String::New("Expected grammar start to be a string")));
  string start_rule_name = StringFromJsString(start);

  Handle<Object> js_rules = ObjectGet<Object>(js_grammar, "rules");
  if (!js_rules->IsObject())
    ThrowException(Exception::TypeError(String::New("Expected grammar rules to be an object")));
  map<const string, const rule_ptr> rules;
  Local<Array> rule_names = js_rules->GetOwnPropertyNames();
  uint32_t length = rule_names->Length();
  for (uint32_t i = 0; i < length; i++) {
    Local<String> js_rule_name = Local<String>::Cast(rule_names->Get(i));
    string rule_name = StringFromJsString(js_rule_name);
    rule_ptr rule = RuleFromJsRule(Handle<Object>::Cast(js_rules->Get(js_rule_name)));
    rules.insert(pair<const string, const rule_ptr>(rule_name, rule));
  }

  return Grammar(start_rule_name, rules);
}

Handle<Value> Compile(const Arguments &args) {
  HandleScope scope;

  Handle<Object> js_grammar = Handle<Object>::Cast(args[0]);
  if (!js_grammar->IsObject())
    ThrowException(Exception::TypeError(String::New("Expected grammar to be an object")));

  Handle<String> js_name = ObjectGet<String>(js_grammar, "name");
  if (!js_name->IsString())
    ThrowException(Exception::TypeError(String::New("Expected grammar name to be a string")));
  string name =  StringFromJsString(js_name);

  Grammar grammar = GrammarFromJsGrammar(js_grammar);
  string code = tree_sitter::compile(grammar, name);
  return scope.Close(String::New(code.c_str()));
}
