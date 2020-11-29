class CubeCoord
  attr_reader :x, :y, :z
  def initialize(x=0, y=0, z=0)
    @x = x
    @y = y
    @z = z
  end

  def +(c)
    CubeCoord.new(@x + c.x, @y + c.y, @z + c.z)
  end

  def <=>(c)
    distance <=> c.distance
  end

  def distance
    (@x.abs + @y.abs + @z.abs) / 2
  end

  def self.from(str)
    case str.strip
    when "n"
      self.new(0, -1, 1)
    when "ne"
      self.new(1, -1, 0)
    when "se"
      self.new(1, 0, -1)
    when "s"
      self.new(0, 1, -1)
    when "sw"
      self.new(-1, 1, 0)
    when "nw"
      self.new(-1, 0, 1)
    end
  end
end


readline
  .split(",")
  .map { |s| CubeCoord.from s }
  .reduce([CubeCoord.new, CubeCoord.new]) { |(c, m), s| [c + s, [m, c + s].max] }
  .each { |x| puts x.distance }
