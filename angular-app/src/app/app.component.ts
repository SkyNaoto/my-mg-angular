import { Component, inject, signal } from '@angular/core';
import { CommonModule } from '@angular/common';
import { HttpClientModule } from '@angular/common/http';
import { ProductService, Product } from './product.service';

@Component({
  selector: 'app-root',
  standalone: true,
  imports: [CommonModule, HttpClientModule],
  templateUrl: './app.component.html',
})
export class AppComponent {
  private productService = inject(ProductService);

  products = signal<Product[]>([]);
  selectedProduct = signal<Product | null>(null);

  loadProducts() {
    this.productService.getProducts().subscribe(data => {
      this.products.set(data);
      this.selectedProduct.set(null);
    });
  }

  loadProductDetail(id: number) {
    this.productService.getProductById(id).subscribe(data => {
      this.selectedProduct.set(data);
    });
  }
}
