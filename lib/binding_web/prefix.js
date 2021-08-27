var TreeSitter = function() {
  class Parser {
    constructor() {
      this.initialize();
    }

    initialize() {
      throw new Error("cannot construct a Parser before calling `init()`");
    }

    static init(treeSitterInitOptions = {}) {
      return new Promise((resolveInitPromise) => {
