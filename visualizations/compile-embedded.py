"""
Compiles the parts of the visualization into one HTML file for
easy embedding.
"""
import os
import sys
import string

current_path = os.path.dirname(__file__)
js_files = [
  'utilities.js',
  'models.js',
  'collections.js',
  'routers.js',
  'views.js',
  'application.js',
]
css_files = [
  'styles.css'
]
output_file = 'index-embeddable.html'
input_file = 'index.html'
js_output = ''
css_output = ''
js_start_delimiter = '<!-- START: To be inline JS -->'
js_end_delimiter = '<!-- END: To be inline JS -->'
css_start_delimiter = '<!-- START: To be inline CSS -->'
css_end_delimiter = '<!-- END: To be inline CSS -->'

# Get JS data
for js in js_files:
  f = open(os.path.join(current_path, 'js', js), 'r')
  js_output = js_output + '\n\n' + f.read()
  f.close()
js_output = '\n<script type="text/javascript">\n%s\n</script>\n' % js_output

# Get CSS dats
for css in css_files:
  f = open(os.path.join(current_path, 'css', css), 'r')
  css_output = css_output + '\n\n' + f.read()
  f.close()
css_output = '\n<style type="text/css">\n%s\n</style>\n' % css_output

# Get input file
input_f = open(os.path.join(current_path, input_file), 'r')
input_content = input_f.read()
input_f.close()

# Replace CSS
start = input_content.find(css_start_delimiter)
start = start + len(css_start_delimiter)
end = input_content.find(css_end_delimiter)
replace_string = input_content[start:end]
output_content = input_content.replace(replace_string, css_output)

# Replace JS
start = input_content.find(js_start_delimiter)
start = start + len(js_start_delimiter)
end = input_content.find(js_end_delimiter)
replace_string = input_content[start:end]
output_content = output_content.replace(replace_string, js_output)

# Write file
output_f = open(os.path.join(current_path, output_file), 'w')
output_f.write(output_content)
output_f.close()