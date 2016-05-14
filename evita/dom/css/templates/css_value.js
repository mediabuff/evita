// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('css.Value');

goog.require('css.Property');

goog.scope(function() {

/** @constructor */
const Property = css.Property;

/** @const @type {string} */
const NUMBER = '[-+]?\\d*[.]?\\d+(?:[Ee][-+]?\\d+)?';

/** @const @type {!Array<string>} */
const ABSOLUTE_LENGTH_UNITS = ['cm', 'mm', 'in', 'pt', 'pt', 'pc', 'px', 'q'];

/** @const @type {!Array<string>} */
const ANGLE_UNITS = ['deg', 'grad', 'rad', 'turn'];

/** @const @type {!Array<string>} */
const DURATION_UNITS = ['ms', 's'];

/** @const @type {!Array<string>} */
const FONT_RELATIVE_LENGTH_UNITS = ['ch', 'em', 'ex', 'rem'];

/** @const @type {!Array<string>} */
const FREQUENCY_UNITS = ['Hz', 'KHz'];

/** @const @type {!Array<string>} */
const RESOLUTION_UNITS = ['dpi', 'dpcm', 'dppx'];

/** @const @type {!Array<string>} */
const VIEWPORT_PERCENTAGE__UNITS = ['vh', 'vmin', 'vmax', 'vw'];


/** @const @type {!Array<string>} */
const LENGTH_UNITS = [].concat(ABSOLUTE_LENGTH_UNITS,
                               FONT_RELATIVE_LENGTH_UNITS);

/** @const @type {!Array<string>} */
const UNITS = [].concat(ABSOLUTE_LENGTH_UNITS, ANGLE_UNITS,
                        DURATION_UNITS, FONT_RELATIVE_LENGTH_UNITS,
                        FREQUENCY_UNITS, RESOLUTION_UNITS,
                        VIEWPORT_PERCENTAGE__UNITS);

/** @const @type {!RegExp} */
const RE_DIMENSION = new RegExp(`^${NUMBER}(?:${UNITS.join('|')})?$`);

/** @const @type {!RegExp} */
const RE_LENGTH = new RegExp(`^${NUMBER}(?:${LENGTH_UNITS.join('|')})?$`);

/** @const @type {!RegExp} */
const RE_NUMBER = new RegExp(`^${NUMBER}$`);

/** @const @type {!RegExp} */
const RE_PERCENTAGE = new RegExp(`^${NUMBER}%$`);

/** @const @type {!RegExp} */
const RE_COLOR = new RegExp('^\\w+\\(.*?\\)$');

/**
 * @const @type {!Set<string>}
 * Note: This table should be matched with C++ one.
 */
const COLOR_NAMES = new Set([
  'currentcolor',
  'aliceblue',  // #F0F8FF 240,248,255
  'antiquewhite',  // #FAEBD7 250,235,215
  'aqua',  // #00FFFF 255,255
  'aquamarine',  // #7FFFD4 127,255,212
  'azure',  // #F0FFFF 240,255,255
  'beige',  // #F5F5DC 245,245,220
  'bisque',  // #FFE4C4 255,228,196
  'black',  // #000000 0,0,0
  'blanchedalmond',  // #FFEBCD 255,235,205
  'blue',  // #0000FF 0,0,255
  'blueviolet',  // #8A2BE2 138,43,226
  'brown',  // #A52A2A 165,42,42
  'burlywood',  // #DEB887 222,184,135
  'cadetblue',  // #5F9EA0 95,158,160
  'chartreuse',  // #7FFF00 127,255,0
  'chocolate',  // #D2691E 210,105,30
  'coral',  // #FF7F50 255,127,80
  'cornflowerblue',  // #6495ED 100,149,237
  'cornsilk',  // #FFF8DC 255,248,220
  'crimson',  // #DC143C 220,20,60
  'cyan',  // #00FFFF 255,255
  'darkblue',  // #00008B 0,0,139
  'darkcyan',  // #008B8B 139,139
  'darkgoldenrod',  // #B8860B 184,134,11
  'darkgray',  // #A9A9A9 169,169,169
  'darkgreen',  // #006400 0,100,0
  'darkgrey',  // #A9A9A9 169,169,169
  'darkkhaki',  // #BDB76B 189,183,107
  'darkmagenta',  // #8B008B 139,0,139
  'darkolivegreen',  // #556B2F 85,107,47
  'darkorange',  // #FF8C00 255,140,0
  'darkorchid',  // #9932CC 153,50,204
  'darkred',  // #8B0000 139,0,0
  'darksalmon',  // #E9967A 233,150,122
  'darkseagreen',  // #8FBC8F 143,188,143
  'darkslateblue',  // #483D8B 72,61,139
  'darkslategray',  // #2F4F4F 47,79,79
  'darkslategrey',  // #2F4F4F 47,79,79
  'darkturquoise',  // #00CED1 206,209
  'darkviolet',  // #9400D3 148,0,211
  'deeppink',  // #FF1493 255,20,147
  'deepskyblue',  // #00BFFF 191,255
  'dimgray',  // #696969 105,105,105
  'dimgrey',  // #696969 105,105,105
  'dodgerblue',  // #1E90FF 30,144,255
  'firebrick',  // #B22222 178,34,34
  'floralwhite',  // #FFFAF0 255,250,240
  'forestgreen',  // #228B22 34,139,34
  'fuchsia',  // #FF00FF 255,0,255
  'gainsboro',  // #DCDCDC 220,220,220
  'ghostwhite',  // #F8F8FF 248,248,255
  'gold',  // #FFD700 255,215,0
  'goldenrod',  // #DAA520 218,165,32
  'gray',  // #808080 128,128,128
  'green',  // #008000 0,128,0
  'greenyellow',  // #ADFF2F 173,255,47
  'grey',  // #808080 128,128,128
  'honeydew',  // #F0FFF0 240,255,240
  'hotpink',  // #FF69B4 255,105,180
  'indianred',  // #CD5C5C 205,92,92
  'indigo',  // #4B0082 75,0,130
  'ivory',  // #FFFFF0 255,255,240
  'khaki',  // #F0E68C 240,230,140
  'lavender',  // #E6E6FA 230,230,250
  'lavenderblush',  // #FFF0F5 255,240,245
  'lawngreen',  // #7CFC00 124,252,0
  'lemonchiffon',  // #FFFACD 255,250,205
  'lightblue',  // #ADD8E6 173,216,230
  'lightcoral',  // #F08080 240,128,128
  'lightcyan',  // #E0FFFF 224,255,255
  'lightgoldenrodyellow',  // #FAFAD2 250,250,210
  'lightgray',  // #D3D3D3 211,211,211
  'lightgreen',  // #90EE90 144,238,144
  'lightgrey',  // #D3D3D3 211,211,211
  'lightpink',  // #FFB6C1 255,182,193
  'lightsalmon',  // #FFA07A 255,160,122
  'lightseagreen',  // #20B2AA 32,178,170
  'lightskyblue',  // #87CEFA 135,206,250
  'lightslategray',  // #778899 119,136,153
  'lightslategrey',  // #778899 119,136,153
  'lightsteelblue',  // #B0C4DE 176,196,222
  'lightyellow',  // #FFFFE0 255,255,224
  'lime',  // #00FF00 0,255,0
  'limegreen',  // #32CD32 50,205,50
  'linen',  // #FAF0E6 250,240,230
  'magenta',  // #FF00FF 255,0,255
  'maroon',  // #800000 128,0,0
  'mediumaquamarine',  // #66CDAA 102,205,170
  'mediumblue',  // #0000CD 0,0,205
  'mediumorchid',  // #BA55D3 186,85,211
  'mediumpurple',  // #9370DB 147,112,219
  'mediumseagreen',  // #3CB371 60,179,113
  'mediumslateblue',  // #7B68EE 123,104,238
  'mediumspringgreen',  // #00FA9A 250,154
  'mediumturquoise',  // #48D1CC 72,209,204
  'mediumvioletred',  // #C71585 199,21,133
  'midnightblue',  // #191970 25,25,112
  'mintcream',  // #F5FFFA 245,255,250
  'mistyrose',  // #FFE4E1 255,228,225
  'moccasin',  // #FFE4B5 255,228,181
  'navajowhite',  // #FFDEAD 255,222,173
  'navy',  // #000080 0,0,128
  'oldlace',  // #FDF5E6 253,245,230
  'olive',  // #808000 128,128,0
  'olivedrab',  // #6B8E23 107,142,35
  'orange',  // #FFA500 255,165,0
  'orangered',  // #FF4500 255,69,0
  'orchid',  // #DA70D6 218,112,214
  'palegoldenrod',  // #EEE8AA 238,232,170
  'palegreen',  // #98FB98 152,251,152
  'paleturquoise',  // #AFEEEE 175,238,238
  'palevioletred',  // #DB7093 219,112,147
  'papayawhip',  // #FFEFD5 255,239,213
  'peachpuff',  // #FFDAB9 255,218,185
  'peru',  // #CD853F 205,133,63
  'pink',  // #FFC0CB 255,192,203
  'plum',  // #DDA0DD 221,160,221
  'powderblue',  // #B0E0E6 176,224,230
  'purple',  // #800080 128,0,128
  'rebeccapurple',  // #663399 102,51,153
  'red',  // #FF0000 255,0,0
  'rosybrown',  // #BC8F8F 188,143,143
  'royalblue',  // #4169E1 65,105,225
  'saddlebrown',  // #8B4513 139,69,19
  'salmon',  // #FA8072 250,128,114
  'sandybrown',  // #F4A460 244,164,96
  'seagreen',  // #2E8B57 46,139,87
  'seashell',  // #FFF5EE 255,245,238
  'sienna',  // #A0522D 160,82,45
  'silver',  // #C0C0C0 192,192,192
  'skyblue',  // #87CEEB 135,206,235
  'slateblue',  // #6A5ACD 106,90,205
  'slategray',  // #708090 112,128,144
  'slategrey',  // #708090 112,128,144
  'snow',  // #FFFAFA 255,250,250
  'springgreen',  // #00FF7F 255,127
  'steelblue',  // #4682B4 70,130,180
  'tan',  // #D2B48C 210,180,140
  'teal',  // #008080 128,128
  'thistle',  // #D8BFD8 216,191,216
  'tomato',  // #FF6347 255,99,71
  'turquoise',  // #40E0D0 64,224,208
  'violet',  // #EE82EE 238,130,238
  'wheat',  // #F5DEB3 245,222,179
  'white',  // #FFFFFF 255,255,255
  'whitesmoke',  // #F5F5F5 245,245,245
  'yellow',  // #FFFF00 255,255,0
  'yellowgreen',  // #9ACD32 154,205,50
]);

/**
 * @param {string} text
 * @return {boolean}
 */
function isColorValue(text) {
  if (text.length < 3)
    return false;
  /** @const @type {number} */
  const charCode = text.charCodeAt(0);
  if (charCode === Unicode.NUMBER_SIGN) {
    // #rgb #rgba #rrggbb #rrggbbaa
    return text.length === 4 || text.length === 5 || text.length === 7 ||
           text.length === 9;
  }
  if (COLOR_NAMES.has(text))
    return true;
  return RE_COLOR.test(text);
}

/**
 * @param {string} text
 * @return {boolean}
 */
function isDimension(text) {
  return RE_DIMENSION.test(text);
}

/**
 * @param {string} text
 * @return {boolean}
 */
function isLength(text) {
  return RE_LENGTH.test(text);
}

/**
 * @param {string} text
 * @return {boolean}
 */
function isPercentage(text) {
  return RE_PERCENTAGE.test(text);
}

/**
 * @param {string} text
 * @return {boolean}
 */
function isNumber(text) {
  return RE_NUMBER.test(text);
}

/**
 * @param {string} text
 * @return {boolean}
 */
function isString(text) { return true; }

class Value {
  /**
   * @param {string} name
   * @param {string} text
   * @return {boolean}
   */
  static isValidFor(text, name) {
    /** @const @type {number} */
    const id = Property.idOf(name);
    if (id < 0)
      return false;
    switch (id) {
{% for property in properties %}
{%  set type = property.type %}
      case {{property.id}}:  // {{property.text}}
{%   if type.is_primitive %}
        return is{{type.Name}}(text);
{%   else %}
{%     for member in type.members if member.is_keyword %}
        if (text === {{member.text}})
          return true;
{%     endfor %}
{%     for member in type.members if not member.is_keyword %}
        if (is{{member.Name}}(text))
          return true;
{%     endfor %}
        return false;
{%   endif %}
{% endfor %}
    }
    throw new Error(`NOTREACHED name='${name}'`);
  }
}

/** @constructor */
css.Value = Value;
});
