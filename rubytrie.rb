class RubyTrie
  def initialize(default = nil)
    @first_child = nil
    @default = default
  end
  
  def [](key)
    raise "key must be a string" unless key.is_a? String
        
    next_node = @first_child
    node = nil
    
    0.upto(key.length - 1) do |i|
      return nil if next_node.nil?
      char = key[i]
      node = next_node.sibling_for_char(char)
      return nil if node.nil?
      next_node = node.first_child
    end
    
    return node.value
  end
  
  def []=(key, val)
    raise "key must be a string" unless key.is_a? String
    
    if @first_child.nil?
      @first_child = TrieNode.new(key[0])
    end
    
    next_node = @first_child
    node = nil
    
    0.upto(key.length - 1) do |i|
      char = key[i]
      node = next_node.sibling_for_char(char)
      node = next_node.add_sibling(char) if node.nil?
      next_node = node.first_child
      next_node = node.add_child(char) if next_node.nil? && i != key.length - 1
    end
    
    node.value = val
    return self
  end
end

class TrieNode
  attr_accessor :char, :next_sibling, :first_child, :value
  
  def initialize(char)
    @next_sibling, @first_child, @value = nil
    @char = char
  end
  
  def sibling_for_char(char)
    node = self
    until node.nil?
      return node if node.char == char
      node = node.next_sibling
    end
    return nil
  end
  
  def add_sibling(char)
    node = self
    
    until node.next_sibling.nil?
      node = node.next_sibling
    end
    node.next_sibling = TrieNode.new(char)
    return node.next_sibling
  end
  
  def add_child(char)
    self.first_child = TrieNode.new(char)
    return self.first_child
  end
end