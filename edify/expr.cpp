
    // Parse up to at least long long or 64-bit integers.
    int64_t l_int;
    if (!android::base::ParseInt(args[0].c_str(), &l_int)) {
        state->errmsg = "failed to parse int in " + args[0];
        return nullptr;
    }

    int64_t r_int;
    if (!android::base::ParseInt(args[1].c_str(), &r_int)) {
        state->errmsg = "failed to parse int in " + args[1];
        return nullptr;
    }

    return StringValue(l_int < r_int ? "t" : "");
}

Value* GreaterThanIntFn(const char* name, State* state,
                        const std::vector<std::unique_ptr<Expr>>& argv) {
    if (argv.size() != 2) {
        state->errmsg = "greater_than_int expects 2 arguments";
        return nullptr;
    }

    std::vector<std::string> args;
    if (!ReadArgs(state, argv, &args)) {
        return nullptr;
    }

    // Parse up to at least long long or 64-bit integers.
    int64_t l_int;
    if (!android::base::ParseInt(args[0].c_str(), &l_int)) {
        state->errmsg = "failed to parse int in " + args[0];
        return nullptr;
    }

    int64_t r_int;
    if (!android::base::ParseInt(args[1].c_str(), &r_int)) {
        state->errmsg = "failed to parse int in " + args[1];
        return nullptr;
    }

    return StringValue(l_int > r_int ? "t" : "");
}

Value* Literal(const char* name, State* state, const std::vector<std::unique_ptr<Expr>>& argv) {
    return StringValue(name);
}

// -----------------------------------------------------------------
//   the function table
// -----------------------------------------------------------------

static std::unordered_map<std::string, Function> fn_table;

void RegisterFunction(const std::string& name, Function fn) {
    fn_table[name] = fn;
}

Function FindFunction(const std::string& name) {
    if (fn_table.find(name) == fn_table.end()) {
        return nullptr;
    } else {
        return fn_table[name];
    }
}

void RegisterBuiltins() {
    RegisterFunction("ifelse", IfElseFn);
    RegisterFunction("abort", AbortFn);
    RegisterFunction("assert", AssertFn);
    RegisterFunction("concat", ConcatFn);
    RegisterFunction("is_substring", SubstringFn);
    RegisterFunction("stdout", StdoutFn);
    RegisterFunction("sleep", SleepFn);

    RegisterFunction("less_than_int", LessThanIntFn);
    RegisterFunction("greater_than_int", GreaterThanIntFn);
}


// -----------------------------------------------------------------
//   convenience methods for functions
// -----------------------------------------------------------------

// Evaluate the expressions in argv, and put the results of strings in args. If any expression
// evaluates to nullptr, return false. Return true on success.
bool ReadArgs(State* state, const std::vector<std::unique_ptr<Expr>>& argv,
              std::vector<std::string>* args) {
    return ReadArgs(state, argv, args, 0, argv.size());
}

bool ReadArgs(State* state, const std::vector<std::unique_ptr<Expr>>& argv,
              std::vector<std::string>* args, size_t start, size_t len) {
    if (args == nullptr) {
        return false;
    }
    if (start + len > argv.size()) {
        return false;
    }
    for (size_t i = start; i < start + len; ++i) {
        std::string var;
        if (!Evaluate(state, argv[i], &var)) {
            args->clear();
            return false;
        }
        args->push_back(var);
    }
    return true;
}

// Evaluate the expressions in argv, and put the results of Value* in args. If any expression
// evaluate to nullptr, return false. Return true on success.
bool ReadValueArgs(State* state, const std::vector<std::unique_ptr<Expr>>& argv,
                   std::vector<std::unique_ptr<Value>>* args) {
    return ReadValueArgs(state, argv, args, 0, argv.size());
}

bool ReadValueArgs(State* state, const std::vector<std::unique_ptr<Expr>>& argv,
                   std::vector<std::unique_ptr<Value>>* args, size_t start, size_t len) {
    if (args == nullptr) {
        return false;
    }
    if (len == 0 || start + len > argv.size()) {
        return false;
    }
    for (size_t i = start; i < start + len; ++i) {
        std::unique_ptr<Value> v(EvaluateValue(state, argv[i]));
        if (!v) {
            args->clear();
            return false;
        }
        args->push_back(std::move(v));
    }
    return true;
}

// Use printf-style arguments to compose an error message to put into
// *state.  Returns nullptr.
Value* ErrorAbort(State* state, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    android::base::StringAppendV(&state->errmsg, format, ap);
    va_end(ap);
    return nullptr;
}

Value* ErrorAbort(State* state, CauseCode cause_code, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    android::base::StringAppendV(&state->errmsg, format, ap);
    va_end(ap);
    state->cause_code = cause_code;
    return nullptr;
}

State::State(const std::string& script, void* cookie) :
    script(script),
    cookie(cookie) {
}
