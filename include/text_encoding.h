#pragma once

/* UTF-8 text becomes Shift-JIS bytes at build time, e.g. _("ゲームデータ").
 * Normal C string termination and fixed-size array initialization rules apply.
 * Adjacent literals are supported. Numeric escapes specify raw bytes; other
 * C escapes retain their usual meaning. Unrepresentable characters fail the
 * build. Character width and Unicode spelling are preserved without normalization.
 */
#define _(text) SKP_ENCODE_SJIS(text)
