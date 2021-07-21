# Heapfrag

This tool allows to measure a fragmentation of the Ruby VM heap.

## Installation

Add this line to your application's Gemfile:

```ruby
gem 'heapfrag'
```

And then execute:

    $ bundle

Or install it yourself as:

    $ gem install heapfrag

## Usage

    > require "heapfrag"
    > Heapfrag::stat

    {
      :fragmented_pages=>146,
      :pages_total=>154,
      :pages_seen=>154,
      :pages_with_alive=>154,
      :pages_with_dead=>146,
      :pages_free=>0,
      :pages_full=>8,
      :objs_alive=>47812,
      :objs_dead=>14961,
                             0% 50% 95%  99% 99.9% 100%
      :heap_pages_fill_cdf=>[0, 22, 116, 133, 142, 154]
    }

## Quick start

    $ rake compile && echo 'require "heapfrag"; Heapfrag::stat' | irb
    install -c tmp/x86_64-linux/heapfrag/2.4.10/heapfrag.so lib/heapfrag/heapfrag.so
    cp tmp/x86_64-linux/heapfrag/2.4.10/heapfrag.so tmp/x86_64-linux/stage/lib/heapfrag/heapfrag.so
    Switch to inspect mode.
    require "heapfrag"; Heapfrag::stat
    {:fragmented_pages=>146, :pages_total=>154, :pages_seen=>154, :pages_with_alive=>154, :pages_with_dead=>146, :pages_free=>0, :pages_full=>8, :objs_alive=>47812, :objs_dead=>14961, :heap_pages_fill_cdf=>[0, 22, 116, 133, 142, 154]}

## Development

After checking out the repo, run `bin/setup` to install dependencies. Then, run `rake spec` to run the tests. You can also run `bin/console` for an interactive prompt that will allow you to experiment.

To install this gem onto your local machine, run `bundle exec rake install`. To release a new version, update the version number in `version.rb`, and then run `bundle exec rake release`, which will create a git tag for the version, push git commits and tags, and push the `.gem` file to [rubygems.org](https://rubygems.org).

## Contributing

Bug reports and pull requests are welcome on GitHub at https://github.com/sitano/heapfrag. This project is intended to be a safe, welcoming space for collaboration, and contributors are expected to adhere to the [Contributor Covenant](http://contributor-covenant.org) code of conduct.

## License

The gem is available as open source under the terms of the [MIT License](https://opensource.org/licenses/MIT).
