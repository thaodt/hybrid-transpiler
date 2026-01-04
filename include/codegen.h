#ifndef HYBRID_CODEGEN_H
#define HYBRID_CODEGEN_H

#include "ir.h"
#include <string>
#include <sstream>

namespace hybrid {

/**
 * Base class for code generators
 */
class CodeGenerator {
public:
    virtual ~CodeGenerator() = default;

    /**
     * Generate code from IR
     * @param ir The intermediate representation
     * @return Generated source code as string
     */
    virtual std::string generate(const IR& ir) = 0;

protected:
    std::stringstream output_;
    int indent_level_ = 0;

    void indent() { indent_level_++; }
    void dedent() { indent_level_--; }
    void writeLine(const std::string& line);
    void writeIndent();
};

/**
 * Rust code generator
 */
class RustCodeGenerator : public CodeGenerator {
public:
    std::string generate(const IR& ir) override;

private:
    void generateClass(const ClassDecl& class_decl);
    void generateFunction(const Function& func, bool is_method = false);
    void generateVariable(const Variable& var);
    void generateTryCatchAsResult(const Function& func);

    // Inheritance/Trait code generation
    void generateTraitImplementations(const ClassDecl& class_decl);
    void generateTraitForBaseClass(const std::string& base_class_name, const ClassDecl& derived_class);

    // Threading code generation
    void generateThreadingCode(const Function& func);
    void generateThreadCreation(const ThreadInfo& thread);
    void generateLockScope(const LockInfo& lock);
    void generateAtomicOperations(const AtomicInfo& atomic);
    void generateConditionVariable(const ConditionVariableInfo& cv);

    // Async/Await code generation
    void generateAsyncFunction(const Function& func);
    void generateCoroutineBody(const Function& func);
    void generateAwaitExpression(const AsyncOperation& op);
    void generateAsyncTask(const AsyncTaskInfo& task);
    void generateFuture(const FutureInfo& future);

    // Template code generation
    std::string convertTemplateParametersToRust(const std::vector<TemplateParameter>& params);
    std::string convertTemplateArgsToRust(const std::vector<TemplateParameter>& params);

    std::string convertType(const std::shared_ptr<Type>& type);
    std::string convertSmartPointer(const std::shared_ptr<Type>& type);
    std::string sanitizeName(const std::string& name);
};

/**
 * Go code generator
 */
class GoCodeGenerator : public CodeGenerator {
public:
    std::string generate(const IR& ir) override;

private:
    void generateClass(const ClassDecl& class_decl);
    void generateFunction(const Function& func, const std::string& receiver_type = "");
    void generateVariable(const Variable& var);
    void generateTryCatchAsError(const Function& func);

    // Inheritance/Interface code generation
    void generateInterfaceImplementations(const ClassDecl& class_decl);
    void generateInterfaceForBaseClass(const std::string& base_class_name, const ClassDecl& derived_class);

    // Threading code generation
    void generateThreadingCode(const Function& func);
    void generateGoroutineCreation(const ThreadInfo& thread);
    void generateMutexLock(const LockInfo& lock);
    void generateAtomicOperations(const AtomicInfo& atomic);
    void generateConditionVariable(const ConditionVariableInfo& cv);

    // Async/Goroutine code generation
    void generateAsyncFunction(const Function& func);
    void generateCoroutineAsGoroutine(const Function& func);
    void generateChannelOperation(const AsyncOperation& op);
    void generateAsyncTask(const AsyncTaskInfo& task);

    std::string convertType(const std::shared_ptr<Type>& type);
    std::string sanitizeName(const std::string& name);
    std::string capitalize(const std::string& name);
};

} // namespace hybrid

#endif // HYBRID_CODEGEN_H
