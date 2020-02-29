void escape(void* p) {
    asm volatile("" : : "g"(p) : "memory");
}