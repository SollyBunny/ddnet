class String
  def snake_to_camel
    self.split('_').map { |w| w.capitalize }.join
  end

  def camel_to_snake
    self.split(/([A-Z][a-z]*)/).each_slice(2).map(&:join).map(&:downcase).join('_')
  end

  def to_camel
    return snake_to_camel if self.match? /_/
    return self.capitalize if self.downcase == self

    self
  end

  def to_snake
    return self if self.match? /_/

    camel_to_snake
  end
end
