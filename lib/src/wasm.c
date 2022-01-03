#ifdef TREE_SITTER_FEATURE_WASM

#include "./wasm/wasm.h"
#include "tree_sitter/api.h"

struct TSWasmContext {
  wasm_store_t *store;
  wasm_extern_vec_t imports;
};

static wasm_trap_t *advance_callback(
  const wasm_val_vec_t *args,
  wasm_val_vec_t *results
) {
  return NULL; 
}

static wasm_trap_t *mark_end_callback(
  const wasm_val_vec_t *args,
  wasm_val_vec_t *results
) {
  return NULL; 
}

static wasm_trap_t *get_column_callback(
  const wasm_val_vec_t *args,
  wasm_val_vec_t *results
) {
  return NULL; 
}

static wasm_trap_t *is_at_included_range_start_callback(
  const wasm_val_vec_t *args,
  wasm_val_vec_t *results
) {
  return NULL; 
}

static wasm_trap_t *eof_callback(
  const wasm_val_vec_t *args,
  wasm_val_vec_t *results
) {
  return NULL; 
}

typedef struct {
  wasm_func_callback_t callback;
  wasm_functype_t *type;
} FunctionDefinition;

TSWasmContext *ts_wasm_context_new(TSWasmEngine *_engine) {
  wasm_engine_t *engine = (wasm_engine_t *)_engine;
  wasm_store_t *store = wasm_store_new(engine);

  // Lexer functions
  const unsigned definitions_len = 5;
  FunctionDefinition definitions[] = {
    {advance_callback, wasm_functype_new_2_0(wasm_valtype_new_i64(), wasm_valtype_new_i32())},
    {mark_end_callback, wasm_functype_new_1_0(wasm_valtype_new_i64())},
    {get_column_callback, wasm_functype_new_1_1(wasm_valtype_new_i64(), wasm_valtype_new_i32())},
    {is_at_included_range_start_callback, wasm_functype_new_1_1(wasm_valtype_new_i64(), wasm_valtype_new_i32())},
    {eof_callback, wasm_functype_new_1_1(wasm_valtype_new_i64(), wasm_valtype_new_i32())},
  };

  // Global imports
  wasm_globaltype_t *const_i32_type = wasm_globaltype_new(wasm_valtype_new_i32(), WASM_CONST);
  wasm_val_t memory_base_val = WASM_I32_VAL(0);
  wasm_val_t table_base_val = WASM_I32_VAL(definitions_len);
  wasm_global_t *memory_base_global = wasm_global_new(store, const_i32_type, &memory_base_val);
  wasm_global_t *table_base_global = wasm_global_new(store, const_i32_type, &table_base_val);

  // Memory import
  wasm_limits_t limits = {.min = 0, .max = wasm_limits_max_default};
  wasm_memorytype_t *memory_type = wasm_memorytype_new(&limits);
  wasm_memory_t *memory = wasm_memory_new(store, memory_type);

  // Table import
  wasm_tabletype_t *table_type = wasm_tabletype_new(wasm_valtype_new(WASM_FUNCREF), &limits);
  wasm_table_t *table = wasm_table_new(store, table_type, NULL);
  wasm_table_grow(table, definitions_len, NULL);
  for (unsigned i = 0; i < definitions_len; i++) {
    FunctionDefinition *definition = &definitions[i];
    wasm_func_t *func = wasm_func_new(store, definition->type, definition->callback);
    wasm_table_set(table, i, wasm_func_as_ref(func));
    wasm_functype_delete(definition->type);
  }

  wasm_extern_t *imports_list[] = {
    wasm_global_as_extern(memory_base_global),
    wasm_global_as_extern(table_base_global),
    wasm_memory_as_extern(memory),
    wasm_table_as_extern(table),
  };
  wasm_extern_vec_t imports = {.size = 4, .data = imports_list};

  TSWasmContext *result = ts_malloc(sizeof(TSWasmContext));
  result->store = store;
  wasm_extern_vec_copy(&result->imports, &imports);
  return result;
}

void ts_wasm_context_delete(
  TSWasmContext *self
) {
  wasm_store_delete(self->store);
  wasm_extern_vec_delete(&self->imports);
  ts_free(self);
}

const TSLanguage *ts_wasm_context_load_language(
  TSWasmContext *self,
  const char *name,
  uint32_t name_len,
  const char *wasm,
  uint32_t wasm_len
) {
  wasm_byte_vec_t file = {.size = wasm_len, .data = (wasm_byte_t *)wasm};
  wasm_module_t *module = wasm_module_new(self->store, &file);
  wasm_trap_t *trap = NULL;
  printf("wasm_len: %u\n", wasm_len);
  wasm_instance_t *instance = wasm_instance_new(self->store, module, &self->imports, &trap);
  if (trap) {
    wasm_message_t message;
    wasm_trap_message(trap, &message);
    printf("error loading wasm: %s\n", message.data);
    abort();
  }
  assert(trap == NULL);

  wasm_extern_vec_t exports;
  wasm_instance_exports(instance, &exports);

  wasm_exporttype_vec_t export_types;
  wasm_module_exports(module, &export_types);

  assert(export_types.size == exports.size);
  for (unsigned i = 0; i < exports.size; i++) {
    wasm_extern_t *export = exports.data[i];
    wasm_exporttype_t *export_type = export_types.data[i];
    const wasm_name_t *name = wasm_exporttype_name(export_type);
    printf("name: %.*s\n", (int)name->size, name->data);
  }
  assert(0);
  return NULL;
}

#endif