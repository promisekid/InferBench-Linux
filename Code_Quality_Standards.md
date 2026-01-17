# 代码质量指标 (Code Quality Standards)

## 1. 代码规范 (Style)
- **风格指南**: 遵循 [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)。
  - 类名: `PascalCase`
  - 函数名: `PascalCase`
  - 变量名: `snake_case`
  - 私有成员变量: `snake_case_` (带尾部下划线)
  - 缩进: 2 或 4 空格 (保持一致)
- **格式化**: 建议使用 `.clang-format` 进行自动格式化。

## 2. 文档注释 (Documentation)
- **公共 API**: 所有 `public` 的类、结构体、方法必须包含 Doxygen 风格的中文注释。
- **格式要求**:
  - `/** ... */` 块注释。
  - 使用 `@brief` 简述功能。
  - 使用 `@param` 描述参数。
  - 使用 `@return` 描述返回值。
  - 复杂逻辑需在实现代码中添加行内注释 `//`.

## 3. 内存安全 (Memory Safety)
- **零泄漏**: 必须通过动态分析工具验证。
- **工具**: 
  - CI/CD 流程中需开启 AddressSanitizer (`-fsanitize=address`)。
  - 或使用 Valgrind Memcheck 运行测试。

## 4. 并发安全 (Concurrency)
- **线程安全**: 多线程共享的可变状态必须加锁 (`std::mutex`, `std::atomic`)。
- **无死锁**: 避免在持锁期间调用外部未知回调，遵循锁顺。
