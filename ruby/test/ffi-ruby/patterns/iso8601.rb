require 'grok'
require 'test/unit'

class ISO8601PatternsTest < Test::Unit::TestCase
  def setup
    @grok = Grok.new
    path = "#{File.dirname(__FILE__)}/../../../patterns/base"
    @grok.add_patterns_from_file(path)
    @grok.compile("^%{TIMESTAMP_ISO8601}$")
  end

  def test_iso8601
    times = [
      "2001-01-01T00:00:00",
      "1974-03-02T04:09:09",
      "2010-05-03T08:18:18+00:00",
      "2004-07-04T12:27:27-00:00",
      "2001-09-05T16:36:36+0000",
      "2001-11-06T20:45:45-0000",
      "2001-12-07T23:54:54Z",
      "2001-01-01T00:00:00.123456",
      "1974-03-02T04:09:09.123456",
      "2010-05-03T08:18:18.123456+00:00",
      "2004-07-04T12:27:27.123456-00:00",
      "2001-09-05T16:36:36.123456+0000",
      "2001-11-06T20:45:45.123456-0000",
      "2001-12-07T23:54:54.123456Z",
      "2001-12-07T23:54:60.123456Z", # '60' second is a leap second.
    ]
    times.each do |time|
      match = @grok.match(time)
      assert_not_equal(false, match, "Expected #{time} to match TIMESTAMP_ISO8601")
      assert_equal(time, match.captures["TIMESTAMP_ISO8601"][0])
    end
  end

  def test_iso8601_nomatch
    times = [
      "2001-13-01T00:00:00", # invalid month
      "2001-00-01T00:00:00", # invalid month
      "2001-01-00T00:00:00", # invalid day
      "2001-01-32T00:00:00", # invalid day
      "2001-01-aT00:00:00", # invalid day
      "2001-01-1aT00:00:00", # invalid day
      "2001-01-01Ta0:00:00", # invalid hour
      "2001-01-01T0:00:00", # invalid hour
      "2001-01-01T25:00:00", # invalid hour
      "2001-01-01T01:60:00", # invalid minute
      "2001-01-01T00:aa:00", # invalid minute
      "2001-01-01T00:00:aa", # invalid second
      "2001-01-01T00:00:-1", # invalid second
      "2001-01-01T00:00:61", # invalid second
      "2001-01-01T00:00:00A", # invalid timezone
      "2001-01-01T00:00:00+", # invalid timezone
      "2001-01-01T00:00:00+25", # invalid timezone
      "2001-01-01T00:00:00+2500", # invalid timezone
      "2001-01-01T00:00:00+25:00", # invalid timezone
      "2001-01-01T00:00:00-25", # invalid timezone
      "2001-01-01T00:00:00-2500", # invalid timezone
      "2001-01-01T00:00:00-00:61", # invalid timezone
    ]
    times.each do |time|
      match = @grok.match(time)
      assert_equal(false, match, "Expected #{time} to not match TIMESTAMP_ISO8601")
    end
  end

end
