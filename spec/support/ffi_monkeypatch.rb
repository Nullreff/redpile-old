class FFI::MemoryPointer
  def read_enum(enum)
    enum[read_int]
  end
end
