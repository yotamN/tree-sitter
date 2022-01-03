use super::{ffi, Language};
use std::os::raw::c_char;

pub use wasmtime;
use wasmtime::Engine;

#[cfg(feature = "wasm")]
pub fn test() {
    wasmtime_c_api::wasm_engine_new();
}

#[repr(C)]
#[derive(Clone)]
pub struct wasm_engine_t {
    pub(crate) engine: Engine,
}

pub struct WasmContext(*mut ffi::TSWasmContext);

impl WasmContext {
    pub fn new(engine: Engine) -> Self {
        let mut c_engine = wasm_engine_t { engine };
        let c_engine = &mut c_engine as *mut _;
        WasmContext(unsafe { ffi::ts_wasm_context_new(c_engine as *mut _) })
    }

    pub fn load_language(&mut self, name: &str, bytes: &[u8]) -> Language {
        Language(unsafe {
            ffi::ts_wasm_context_load_language(
                self.0,
                name.as_ptr() as *const c_char,
                name.len() as u32,
                bytes.as_ptr() as *const c_char,
                bytes.len() as u32,
            )
        })
    }
}

impl Drop for WasmContext {
    fn drop(&mut self) {
        unsafe { ffi::ts_wasm_context_delete(self.0) };
    }
}
