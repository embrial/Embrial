import XCTest
import SwiftTreeSitter
import TreeSitterEmbrial

final class TreeSitterEmbrialTests: XCTestCase {
    func testCanLoadGrammar() throws {
        let parser = Parser()
        let language = Language(language: tree_sitter_embrial())
        XCTAssertNoThrow(try parser.setLanguage(language),
                         "Error loading Embrial grammar")
    }
}
