require 'trie'
require 'rubytrie'

max = 1000000

def time
  starttime = Time.now
  yield
  endtime = Time.now
  puts "Time elapsed: #{endtime - starttime} seconds"
end

puts "With a native Trie..."
t = Trie.new
time do
  1.upto(max) do |i|
    t["item #{i}"] = "sweet"
  end

  1.upto(max) do |i|
    t["item #{i}"].class
  end
  
  1.upto(max) do |i|
    t.delete("item #{i}")
  end
end


puts "With a Hash..."
t = Hash.new
time do
  1.upto(max) do |i|
    t["item #{i}"] = "sweet"
  end

  1.upto(max) do |i|
    t["item #{i}"].class
  end

  1.upto(max) do |i|
    t.delete("item #{i}")
  end
end

# puts "With a Ruby Trie..."
# t = RubyTrie.new
# time do
#   1.upto(max) do |i|
#     t["item #{i}"] = "sweet"
#   end
# 
#   1.upto(max) do |i|
#     t["item #{i}"].class
#   end
# end

