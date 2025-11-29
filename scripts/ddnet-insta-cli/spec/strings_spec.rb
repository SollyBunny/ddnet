# frozen_string_literal: true

require_relative '../lib/strings'

describe 'String', :array do
  context 'casing' do
    it 'Should do camel to snake' do
      expect("HelloWorld".camel_to_snake).to eq("hello_world")
    end
    it 'Should do snake to camel' do
      expect("hello_world".snake_to_camel).to eq("HelloWorld")
    end
    it 'Should do any to snake' do
      expect("HelloWorld".to_snake).to eq("hello_world")
      expect("hello_world".to_snake).to eq("hello_world")
      expect("hello".to_snake).to eq("hello")
    end
    it 'Should do any to camel' do
      expect("HelloWorld".to_camel).to eq("HelloWorld")
      expect("hello_world".to_camel).to eq("HelloWorld")
      expect("hello".to_camel).to eq("Hello")
    end
  end
end
