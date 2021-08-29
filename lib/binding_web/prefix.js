var TreeSitter = function() {
  var initPromise;
  class Parser {
    constructor() {
      this.initialize();
    }

    initialize() {
      throw new Error("cannot construct a Parser before calling `init()`");
    }

    static init(treeSitterInitOptions = {}) {
      if (initPromise) return initPromise;
      return initPromise = new Promise((resolveInitPromise) => {
