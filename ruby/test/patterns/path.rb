#require 'rubygems'
require 'Grok'
require 'test/unit'

class PathPatternsTest < Test::Unit::TestCase
  def setup
    @grok = Grok.new
    path = "#{File.dirname(__FILE__)}/../../../patterns/base"
    @grok.add_patterns_from_file(path)
    @grok.compile("%{PATH}")
  end

  def test_unix_paths
    paths = %w{/ /usr /usr/bin /usr/bin/foo /etc/motd /home/.test
               /foo/bar//baz //testing /.test /%foo% /asdf/asdf,v}
    paths.each do |path|
      match = @grok.match(path)
      assert_not_equal(false, match)
      assert_equal(path, match.captures["PATH"][0])
    end
  end

  def test_windows_paths
    paths = %w{C:\WINDOWS \\Foo\bar}
    paths << "C:\\Documents and Settings\\"
    paths.each do |path|
      match = @grok.match(path)
      assert_not_equal(false, match)
      assert_equal(path, match.captures["PATH"][0])
    end
  end
end
