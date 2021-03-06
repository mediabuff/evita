# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of self source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from pattern_lexer import PatternLexer
from pattern_nfa import NfaEdge, NfaGraph, NfaState


class NfaBuilder(object):

    def __init__(self):
        self._states = []
        self._group_name = ''
        self._initial_states = []

    def build(self, group_name, node):
        self._group_name = group_name
        from_state = self._new_state(None)
        self._initial_states.append(from_state)
        self._process(from_state, None, node)

    def finalize(self):
        if len(self._initial_states) == 1:
            return NfaGraph(self._states, self._initial_states[0])
        initial_state = self._new_state(None)
        for state in self._initial_states:
            self._make_edge(None, initial_state, state)
        return NfaGraph(self._states, initial_state)

    def _make_edge(self, label, from_state, to_state, is_lazy=False):
        if to_state == None:
            to_state = self._new_state(None)
        edge = NfaEdge(label, from_state, to_state, is_lazy)
        from_state.add_out_edge(edge)
        to_state.add_in_edge(edge)
        return to_state

    def _new_state(self, node):
        state = NfaState(self._group_name, len(self._states) + 1, node)
        self._states.append(state)
        return state

    def _process(self, from_state, to_state, node, is_lazy=False):
        if node.is_primary:
            return self._make_edge(node.token, from_state, to_state, is_lazy)
        if node.is_or:
            return self._process_or(from_state, to_state, node, is_lazy)
        if node.is_repeat:
            return self._process_repeat(from_state, to_state, node, is_lazy)
        if node.is_sequence:
            return self._process_sequence(from_state, to_state, node, is_lazy)
        raise Exception('Bad node %s' % str(node))

    def _process_or(self, from_state, to_state, node, is_lazy):
        for member in node.members:
            to_state = self._process(from_state, to_state, member, is_lazy)
        return to_state

    def _process_repeat(self, from_state, to_state, node, is_lazy):
        assert node.max_count > 0, str(node)
        for index in xrange(0, node.min_count):  # pylint: disable=W0612
            from_state = self._process(from_state, None, node.expression,
                                       is_lazy)
        is_lazy = node.is_lazy
        if node.is_infinity:
            # Make two edges for |from_state|, one is empty transition
            # to |to_state| and another is |node.expression| to
            # |from_state|.
            repeat_end = self._process(from_state, from_state,
                                       node.expression, is_lazy)
            return self._make_edge(None, repeat_end, to_state, is_lazy)
        # Make two edges for |from_state| to |to_state|, one is empty
        # transition and another is |node.expression|.
        for index in xrange(1, node.max_count):
            to_state = self._make_edge(None, from_state, to_state, is_lazy)
            from_state = self._process(from_state, None, node.expression,
                                       is_lazy)
        to_state = self._make_edge(None, from_state, to_state, is_lazy)
        return self._process(from_state, to_state, node.expression, is_lazy)

    def _process_sequence(self, from_state, to_state, node, is_lazy):
        index = 0
        for member in node.members:
            index = index + 1
            if index == len(node.members):
                return self._process(from_state, to_state, member, is_lazy)
            from_state = self._process(from_state, None, member, is_lazy)
            if is_lazy:
                continue
            for edge in from_state.in_edges:
                if edge.is_lazy:
                    is_lazy = True
                    break
        assert(False)
