#!/usr/bin/env ruby
require 'erb'
require 'kramdown'

template = ERB.new(DATA.read)

Dir.glob(File.join(File.dirname(__FILE__), '*.html')) {|file| File.delete(file)}
Dir.glob(File.join(File.dirname(__FILE__), '*.md')).each do |file|
  markdown = File.read(file).force_encoding('UTF-8')
  markdown.gsub!(/\(([A-Za-z0-9-]+)\.md\)/, '(\1.html)')

  @title = 'Redpile'
  @body = Kramdown::Document.new(markdown).to_html

  File.write(file.gsub('.md', '.html'), template.result)
end

__END__
<!doctype html>
<html>
  <head>
    <meta charset="utf-8">
    <title><%= @title %></title>
    <link href="//maxcdn.bootstrapcdn.com/bootstrap/3.2.0/css/bootstrap.min.css" rel="stylesheet">
  </head>
  <body>
    <div class="container">
      <ul class="nav nav-pills">
        <li><a href="/">Home</a></li>
        <li><a href="/configuration.html">Configuration</a></li>
        <li><a href="/commands.html">Commands</a></li>
        <li><a href="/types.html">Types</a></li>
        <li><a href="https://github.com/nullreff/redpile">Code</a></li>
      </ul>
      <div class="content">
        <%= @body %>
      </div>
    </div>
  </body>
</html>

